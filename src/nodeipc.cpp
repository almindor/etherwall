    /*
    This file is part of etherwall.
    etherwall is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    etherwall is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with etherwall. If not, see <http://www.gnu.org/licenses/>.
*/
/** @file etheripc.cpp
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Ethereum IPC client implementation
 */

#include "nodeipc.h"
#include "helpers.h"
#include <QSettings>
#include <QFileInfo>
#include <QtConcurrent/QtConcurrent>

// windblows hacks coz windblows sucks
#ifdef Q_OS_WIN32
    #define _WIN32_WINNT 0x0502
    #include <windows.h>
#endif

namespace Etherwall {

// *************************** NodeRequestIPC **************************** //
    int NodeRequest::sCallID = 0;

    NodeRequest::NodeRequest(NodeRequestBurden burden, NodeRequestTypes type, const QString method, const QJsonArray params, int index) :
        fCallID(sCallID++), fType(type), fMethod(method), fParams(params), fIndex(index), fBurden(burden)
    {
    }

    NodeRequest::NodeRequest(NodeRequestTypes type, const QString method, const QJsonArray params, int index) :
        fCallID(sCallID++), fType(type), fMethod(method), fParams(params), fIndex(index), fBurden(Full)
    {
    }

    NodeRequest::NodeRequest(NodeRequestBurden burden) : fBurden(burden)
    {
    }

    NodeRequestTypes NodeRequest::getType() const {
        return fType;
    }

    const QString& NodeRequest::getMethod() const {
        return fMethod;
    }

    const QJsonArray& NodeRequest::getParams() const {
        return fParams;
    }

    int NodeRequest::getIndex() const {
        return fIndex;
    }

    const QVariantMap NodeRequest::getUserData() const
    {
        return fUserData;
    }

    void NodeRequest::setUserData(const QVariantMap &data)
    {
        fUserData = data;
    }

    int NodeRequest::getCallID() const {
        return fCallID;
    }

    NodeRequestBurden NodeRequest::burden() const {
        return fBurden;
    }

// *************************** NodeIPC **************************** //

#ifdef Q_OS_WIN32
    const QString NodeIPC::sDefaultDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Ethereum";
#else
    #ifdef Q_OS_MACX
    const QString NodeIPC::sDefaultDataDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Library/Ethereum";
    #else
    const QString NodeIPC::sDefaultDataDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.ethereum";
    #endif
#endif
    const QString NodeIPC::sDefaultGethArgs = "--syncmode=fast --cache 512";

    NodeIPC::NodeIPC(GethLog& gethLog) :
        fPath(), fBlockFilterID(), fClosingApp(false), fPeerCount(0), fActiveRequest(None),
        fGeth(), fStarting(0), fGethLog(gethLog),
        fSyncing(false), fCurrentBlock(0), fHighestBlock(0), fStartingBlock(0),
        fConnectAttempts(0), fKillTime(), fExternal(false), fEventFilterIDs(),
        fReceivedMsg()
    {
        connect(&fSocket, (void (QLocalSocket::*)(QLocalSocket::LocalSocketError))&QLocalSocket::error, this, &NodeIPC::onSocketError);
        connect(&fSocket, &QLocalSocket::readyRead, this, &NodeIPC::onSocketReadyRead);
        connect(&fSocket, &QLocalSocket::connected, this, &NodeIPC::connectedToServer);
        connect(&fSocket, &QLocalSocket::disconnected, this, &NodeIPC::disconnectedFromServer);
        connect(&fGeth, &QProcess::started, this, &NodeIPC::connectToServer);
        connect(this, &NodeIPC::ipcReady, this, &NodeIPC::onIpcReady);
        connect(this, &NodeIPC::requestDone, this, &NodeIPC::onRequestDone);
        connect(this, &NodeIPC::stopTimer, this, &NodeIPC::onStopTimer);

        const QSettings settings;

        connect(&fTimer, &QTimer::timeout, this, &NodeIPC::onTimer);
    }

    NodeIPC::~NodeIPC() {
        fGeth.kill();
    }

    void NodeIPC::start(const QString &version, const QString &endpoint, const QString &warning)
    {
        Q_UNUSED(version); // TODO
        Q_UNUSED(endpoint);
        Q_UNUSED(warning);

        buildGethArgs(); // has to run first

        fConnectAttempts = 0;
        if ( fStarting <= 0 ) { // try to connect without starting geth
            EtherLog::logMsg("Etherwall starting", LS_Info);
            fStarting = 1;
            emit startingChanged(fStarting);
            return connectToServer();
        }

        init();
    }

    void NodeIPC::init() {
        QSettings settings;       
        QStringList args = buildGethArgs(); // has to run first
        const QString progStr = settings.value("geth/path", defaultGethPath()).toString();

        QFileInfo info(progStr);
        if ( !info.exists() || !info.isExecutable() ) {
            fStarting = -1;
            emit startingChanged(-1);
            setError("Could not find Geth. Please check Geth path and try again.");
            return bail();
        }

        EtherLog::logMsg("Geth starting " + progStr + " " + args.join(" "), LS_Info);
        fStarting = 2;

        fGethLog.attach(&fGeth);
        fGeth.start(progStr, args);
        emit startingChanged(0);
    }

    void NodeIPC::waitConnect() {
        if ( fStarting > 2 ) {
            return;
        }

        // didn't connect to geth in time, run it now
        if ( fStarting == 1 ) {
            fSocket.abort(); // ensure we don't get connected called later
            return init();
        }

        if ( fStarting != 1 && fStarting != 2 ) {
            return connectionTimeout();
        }

        if ( ++fConnectAttempts < 20 ) {
            if ( fSocket.state() == QLocalSocket::ConnectingState ) {
                fSocket.abort();
            }

            connectToServer();
        } else {
            connectionTimeout();
        }
    }

    void NodeIPC::connectToServer() {
        fActiveRequest = NodeRequest(Full);
        emit busyChanged(getBusy());
        if ( fSocket.state() != QLocalSocket::UnconnectedState ) {
            return QTimer::singleShot(2000, this, SLOT(waitConnect()));
        }

        fSocket.connectToServer(fPath);
        if ( fConnectAttempts == 0 ) {
            if ( fStarting == 1 ) {
                EtherLog::logMsg("Checking to see if there is an already running geth...");
            } else {
                EtherLog::logMsg("Connecting to IPC socket " + fPath);
            }
        }

        QTimer::singleShot(2000, this, SLOT(waitConnect()));
    }

    void NodeIPC::connectedToServer() {
        done();

        if ( fStarting == 1 ) {
            fExternal = true;
            emit externalChanged(true);
            fGethLog.append("Attached to external geth, see logs in terminal window.");
        }
        fStarting = 3;

        EtherLog::logMsg("Connected to IPC socket");
        finishInit();
    }

    void NodeIPC::connectionTimeout() {
        fSocket.abort();
        fStarting = -1;
        emit startingChanged(fStarting);
        setError("Unable to establish IPC connection to Geth. Fix path to Geth and try again.");
        bail();
    }

    bool NodeIPC::getBusy() const {
        return (fActiveRequest.burden() != None);
    }

    bool NodeIPC::getExternal() const {
        return fExternal;
    }

    bool NodeIPC::getStarting() const {
        return (fStarting == 1 || fStarting == 2);
    }

    bool NodeIPC::getClosing() const {
        return fClosingApp;
    }

    const QString& NodeIPC::getError() const {
        return fError;
    }

    int NodeIPC::getCode() const {
        return fCode;
    }


    void NodeIPC::setInterval(int interval) {
        QSettings settings;
        settings.setValue("ipc/interval", (int) (interval / 1000));
        fTimer.setInterval(interval);
    }

    bool NodeIPC::getTestnet() const {
        return fNetVersion == 4;
    }

    const QString NodeIPC::getNetworkPostfix() const {
        return Helpers::networkPostfix(fNetVersion);
    }

    bool NodeIPC::killGeth() {
        if ( fGeth.state() == QProcess::NotRunning ) {
            return true;
        }

        if ( fKillTime.elapsed() == 0 ) {
            fKillTime.start();
#ifdef Q_OS_WIN32
            SetConsoleCtrlHandler(NULL, true);
            AttachConsole(fGeth.processId());
            GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
            FreeConsole();
#else
            fGeth.terminate();
#endif
        } else if ( fKillTime.elapsed() > 6000 ) {
            qDebug() << "Geth did not exit in 6 seconds. Killing...\n";
            fGeth.kill();
            return true;
        }

        return false;
    }

    bool NodeIPC::closeApp() {
        if ( !fClosingApp ) {
            EtherLog::logMsg("Closing etherwall");
            fClosingApp = true;
            fTimer.stop();
            emit closingChanged(true);
        }

        if ( fSocket.state() == QLocalSocket::ConnectedState && getBusy() ) { // wait for operation first if we're still connected
            return false;
        }

        if ( fSocket.state() == QLocalSocket::ConnectedState ) {
            bool removed = false;
            if ( !fBlockFilterID.isEmpty() ) { // remove block filter if still connected
                uninstallFilter(fBlockFilterID);
                fBlockFilterID.clear();
                removed = true;
            }

            if ( !fEventFilterIDs.isEmpty() ) { // remove event filters if still connected
                QMapIterator<QString, QString> i(fEventFilterIDs);
                while (i.hasNext()) {
                    i.next();
                    uninstallFilter(i.key());
                }
                fEventFilterIDs.clear();
                removed = true;
            }

            if ( removed ) {
                return false;
            }
        }

        if ( fSocket.state() != QLocalSocket::UnconnectedState ) { // wait for clean disconnect
            fActiveRequest = NodeRequest(Full);
            fSocket.disconnectFromServer();
            return false;
        }

        return killGeth();
    }

    void NodeIPC::loadLogs(const QStringList& addresses, const QJsonArray& topics, quint64 fromBlock, const QString& internalID) {
        if ( addresses.length() > 0 ) {
            getLogs(addresses, topics, fromBlock, internalID);
        }
    }

    void NodeIPC::disconnectedFromServer() {
        if ( fClosingApp ) { // expected
            return;
        }

        fError = fSocket.errorString();
        bail();
    }

    void NodeIPC::getAccounts() {
        if ( !queueRequest(NodeRequest(GetAccountRefs, "personal_listAccounts", QJsonArray())) ) {
            return bail();
        }
    }

    void NodeIPC::handleGetAccounts() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        QJsonArray refs = jv.toArray();
        QStringList accounts;
        foreach( QJsonValue r, refs ) {
            const QString hash = r.toString("INVALID");
            accounts.append(hash);
        }

        emit getAccountsDone(accounts);
        done();
    }

    bool NodeIPC::refreshAccount(const QString& hash, int index) {
        if ( getBalance(hash, index) ) {
            return getTransactionCount(hash, index);
        }

        return false;
    }

    bool NodeIPC::getBalance(const QString& hash, int index) {
        QJsonArray params;
        params.append(hash);
        params.append(QString("latest"));
        if ( !queueRequest(NodeRequest(GetBalance, "eth_getBalance", params, index)) ) {
            bail();
            return false;
        }

        return true;
    }

    bool NodeIPC::getTransactionCount(const QString& hash, int index) {
        QJsonArray params;
        params.append(hash);
        params.append(QString("latest"));
        if ( !queueRequest(NodeRequest(GetTransactionCount, "eth_getTransactionCount", params, index)) ) {
            bail();
            return false;
        }

        return true;
    }


    void NodeIPC::handleAccountBalance() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        const QString decStr = Helpers::toDecStrEther(jv);
        const int index = fActiveRequest.getIndex();

        emit accountBalanceChanged(index, decStr);
        done();
    }

    void NodeIPC::handleAccountTransactionCount() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        std::string hexStr = jv.toString("0x0").remove(0, 2).toStdString();
        const BigInt::Vin bv(hexStr, 16);
        quint64 count = bv.toUlong();
        const int index = fActiveRequest.getIndex();

        emit accountSentTransChanged(index, count);
        done();
    }

    void NodeIPC::newAccount(const QString& password, int index) {
        QJsonArray params;
        params.append(password);
        if ( !queueRequest(NodeRequest(NewAccount, "personal_newAccount", params, index)) ) {
            return bail();
        }
    }

    void NodeIPC::handleNewAccount() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        const QString result = jv.toString();
        emit newAccountDone(result, fActiveRequest.getIndex());
        done();
    }

    void NodeIPC::getBlockNumber() {
        if ( !queueRequest(NodeRequest(NonVisual, GetBlockNumber, "eth_blockNumber")) ) {
            return bail();
        }
    }

    void NodeIPC::handleGetBlockNumber() {
        quint64 result;
        if ( !readNumber(result) ) {
             return bail();
        }

        fBlockNumber = result;
        emit getBlockNumberDone(result);
        done();
    }

    quint64 NodeIPC::blockNumber() const {
        return fBlockNumber;
    }

    int NodeIPC::network() const
    {
        return fNetVersion;
    }

    quint64 NodeIPC::nonceStart() const
    {
        switch ( fNetVersion ) {
            case 2: return 0x0100000;
            case 3: return 0x0100000;
            case 4: return 0;
        }

        return 0;
    }

    void NodeIPC::getPeerCount() {
        if ( !queueRequest(NodeRequest(NonVisual, GetPeerCount, "net_peerCount")) ) {
            return bail();
        }
    }

    void NodeIPC::handleGetPeerCount() {
        if ( !readNumber(fPeerCount) ) {
             return bail();
        }

        emit peerCountChanged(fPeerCount);
        done();
    }

    void NodeIPC::sendTransaction(const Ethereum::Tx& tx, const QString& password) {
        QJsonArray params;
        QJsonObject p;
        p["from"] = tx.fromStr();
        p["value"] = tx.valueHex();
        if ( tx.hasTo() ) {
            p["to"] = tx.toStr();
        }
        if ( tx.hasDefinedGas() ) {
            p["gas"] = tx.gasHex();
        }
        if ( tx.hasDefinedGasPrice() ) {
            p["gasPrice"] = tx.gasPriceHex();
            EtherLog::logMsg(QString("Trans gasPrice: ") + tx.gasPriceStr() + QString(" HexValue: ") + tx.gasPriceHex());
        }
        if ( tx.hasData() ) {
            p["data"] = tx.dataHex();
        }

        params.append(p);
        params.append(password);

        if ( !queueRequest(NodeRequest(SendTransaction, "personal_signAndSendTransaction", params)) ) {
            return bail(true); // softbail
        }
    }

    void NodeIPC::signTransaction(const Ethereum::Tx &tx, const QString &password)
    {
        qDebug() << "called sendTX with manual signing\n";
        unlockAccount(tx.fromStr(), password, 5, 0);
        signTransaction(tx);
    }

    void NodeIPC::signTransaction(const Ethereum::Tx &tx)
    {
        QJsonArray params;
        QJsonObject p;
        p["from"] = tx.fromStr();
        p["value"] = tx.valueHex();
        if ( tx.hasTo() ) {
            p["to"] = tx.toStr();
        }
        if ( tx.hasDefinedGas() ) {
            p["gas"] = tx.gasHex();
        }
        if ( tx.hasDefinedGasPrice() ) {
            p["gasPrice"] = tx.gasPriceHex();
            EtherLog::logMsg(QString("Trans gasPrice: ") + tx.gasPriceStr() + QString(" HexValue: ") + tx.gasPriceHex());
        }
        if ( tx.hasData() ) {
            p["data"] = tx.dataHex();
        }
        p["nonce"] = tx.nonceHex();

        params.append(p);

        if ( !queueRequest(NodeRequest(SignTransaction, "eth_signTransaction", params)) ) {
            return bail(true); // softbail
        }
    }

    void NodeIPC::sendRawTransaction(const Ethereum::Tx &tx)
    {
        sendRawTransaction(Helpers::hexPrefix(tx.encodeRLP(true)));
    }

    void NodeIPC::sendRawTransaction(const QString &rlp)
    {
        QJsonArray params;
        params.append(rlp);

        if ( !queueRequest(NodeRequest(SendRawTransaction, "eth_sendRawTransaction", params)) ) {
            return bail(true); // softbail
        }
    }

    void NodeIPC::call(const Ethereum::Tx &tx, int index, const QVariantMap& userData)
    {
        QJsonArray params;
        QJsonObject p;
        if ( tx.hasFrom() ) {
            p["from"] = tx.fromStr();
        }
        if ( tx.hasValue() ) {
            p["value"] = tx.valueHex();
        }
        if ( tx.hasTo() ) {
            p["to"] = tx.toStr();
        }
        if ( tx.hasDefinedGas() ) {
            p["gas"] = tx.gasHex();
        }
        if ( tx.hasDefinedGasPrice() ) {
            p["gasPrice"] = tx.gasPriceHex();
            EtherLog::logMsg(QString("Trans gasPrice: ") + tx.gasPriceStr() + QString(" HexValue: ") + tx.gasPriceHex());
        }
        if ( tx.hasData() ) {
            p["data"] = tx.dataHex();
        }

        params.append(p);
        params.append("latest");

        NodeRequest request(Call, "eth_call", params, index);
        request.setUserData(userData);
        if ( !queueRequest(request) ) {
            return bail(true); // softbail
        }
    }

    void NodeIPC::handleSendTransaction() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail(true); // softbail
        }

        const QString hash = jv.toString();
        emit sendTransactionDone(hash);
        done();
    }

    void NodeIPC::handleSignTransaction()
    {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail(true); // softbail
        }

        const QString hash = jv.toObject().value("raw").toString();
        emit signTransactionDone(hash);
        done();
    }

    void NodeIPC::handleCall()
    {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail(true); // softbail
        }

        const QString result = jv.toString("error");
        emit callDone(result, fActiveRequest.getIndex(), fActiveRequest.getUserData());
        done();
    }

    int NodeIPC::getConnectionState() const {
        if ( fSocket.state() == QLocalSocket::ConnectedState ) {
            return 1; // TODO: add higher states per peer count!
        }

        return 0;
    }

    void NodeIPC::getGasPrice() {
        if ( !queueRequest(NodeRequest(NonVisual, GetGasPrice, "eth_gasPrice")) ) {
            return bail();
        }
    }

    void NodeIPC::handleGetGasPrice() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        const QString decStr = Helpers::toDecStrEther(jv);
        emit getGasPriceDone(decStr);
        done();
    }

    quint64 NodeIPC::peerCount() const {
        return fPeerCount;
    }

    void NodeIPC::onIpcReady()
    {
        EtherLog::logMsg("IPC ready, initializing poller", LS_Info);
        const QSettings settings;
        setInterval(settings.value("ipc/interval", 10).toInt() * 1000); // re-set here, used for inheritance purposes
        fTimer.start(); // should happen after filter creation, might need to move into last filter response handler
        // if we connected to external geth, put that info in geth log
        emit startingChanged(fStarting);
        emit connectToServerDone();
        emit connectionStateChanged();
    }

    void NodeIPC::onRequestDone()
    {
        emit requestChanged();
        fActiveRequest = NodeRequest(None);
        if ( !fRequestQueue.isEmpty() ) {
            const NodeRequest request = fRequestQueue.dequeue();
            writeRequest(request);
        } else {
            emit busyChanged(getBusy());
        }
    }

    void NodeIPC::onStopTimer()
    {
        fTimer.stop();
    }

    bool NodeIPC::endpointWritable()
    {
        return fSocket.isWritable();
    }

    qint64 NodeIPC::endpointWrite(const QByteArray &data)
    {
        return fSocket.write(data);
    }

    const QByteArray NodeIPC::endpointRead()
    {
        return fSocket.readAll();
    }

    const QStringList NodeIPC::buildGethArgs()
    {
        QSettings settings;
        QString argStr = settings.value("geth/args", sDefaultGethArgs).toString();
        const QString ddStr = settings.value("geth/datadir", sDefaultDataDir).toString();

        // check deprecated options and replace them
        if ( argStr.contains("--light") || argStr.contains("--fast") ) {
            argStr = argStr.replace("--light", "--syncmode=light");
            argStr = argStr.replace("--fast", "--syncmode=fast");
            settings.setValue("geth/args", argStr);
            qDebug() << "replaced args\n";
        }

        QStringList args;
        bool testnet = settings.value("geth/testnet", false).toBool();
        fPath = defaultIPCPath(ddStr, testnet);
        if ( testnet ) { // geth 1.6.0+ only
            args = (argStr + " --datadir " + ddStr + "/rinkeby").split(' ', QString::SkipEmptyParts);
            args.append("--rinkeby");
        } else {
            args = (argStr + " --datadir " + ddStr).split(' ', QString::SkipEmptyParts);
        }
        args.append("--nousb");

        return args;
    }

    void NodeIPC::finishInit()
    {
        getClientVersion();
        getBlockNumber();
        newBlockFilter();
        getSyncing();
        getNetVersion();
    }

    void NodeIPC::estimateGas(const QString& from, const QString& to, const QString& valStr,
                                   const QString& gas, const QString& gasPrice, const QString& data) {
        const QString valHex = Helpers::toHexWeiStr(valStr);
        QJsonArray params;
        QJsonObject p;
        p["from"] = from;
        p["value"] = valHex;
        if ( !to.isEmpty() ) {
            p["to"] = to;
        }
        if ( !gas.isEmpty() ) {
            const QString gasHex = Helpers::decStrToHexStr(gas);
            p["gas"] = gasHex;
        }
        if ( !gasPrice.isEmpty() ) {
            const QString gasPriceHex = Helpers::toHexWeiStr(gasPrice);
            p["gasPrice"] = gasPriceHex;
        }
        if ( !data.isEmpty() ) {
            p["data"] = Helpers::hexPrefix(data);
        }

        params.append(p);
        if ( !queueRequest(NodeRequest(NonVisual, EstimateGas, "eth_estimateGas", params)) ) {
            return bail();
        }
    }

    void NodeIPC::handleEstimateGas() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail(true); // probably gas too low
        }

        const QString price = Helpers::toDecStr(jv);

        emit estimateGasDone(price);

        done();
    }

    void NodeIPC::newBlockFilter() {
        if ( !fBlockFilterID.isEmpty() ) {
            setError("Filter already set");
            return bail(true);
        }

        if ( !queueRequest(NodeRequest(NewBlockFilter, "eth_newBlockFilter")) ) {
            return bail();
        }
    }

    void NodeIPC::newEventFilter(const QJsonArray& addresses, const QJsonArray& topics, const QString& internalID) {
        QJsonArray params;
        QJsonObject o;
        o["address"] = addresses;
        if ( topics.size() > 0 ) {
            o["topics"] = topics;
        }
        params.append(o);

        NodeRequest request(NewEventFilter, "eth_newFilter", params);
        QVariantMap userData;
        userData["internalID"] = internalID;
        request.setUserData(userData);
        if ( !queueRequest(request) ) {
            return bail();
        }
    }

    void NodeIPC::handleNewBlockFilter() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }
        fBlockFilterID = jv.toString();
        // qDebug() << "new block filter: " << fBlockFilterID << "\n";

        if ( fBlockFilterID.isEmpty() ) {
            setError("Block filter ID invalid");
            return bail();
        }

        done();
    }

    void NodeIPC::handleNewEventFilter() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }
        const QString id = jv.toString();
        if ( id.isEmpty() ) {
            setError("Event filter ID invalid");
            return bail();
        }
        const QString internalID = fActiveRequest.getUserData().value("internalID").toString();
        fEventFilterIDs[internalID] = id;
        // qDebug() << "new event filter: " << id << " internalID: " << internalID << "\n";

        done();
    }

    void NodeIPC::onTimer() {
        getPeerCount();
        getSyncing();

        if ( !fBlockFilterID.isEmpty() && !fSyncing ) {
            getFilterChanges(fBlockFilterID, QString());
        } else {
            getBlockNumber();
        }

        if ( !fEventFilterIDs.isEmpty() ) {
            QMapIterator<QString, QString> i(fEventFilterIDs);
            while ( i.hasNext() ) {
                i.next();
                getFilterChanges(i.value(), i.key());
            }
        }
    }

    int NodeIPC::parseVersionNum() const {
        QRegExp reg("^Geth/v([0-9]+)\\.([0-9]+)\\.([0-9]+).*$");
        reg.indexIn(fClientVersion);
        if ( reg.captureCount() == 3 ) try { // it's geth
            return reg.cap(1).toInt() * 100000 + reg.cap(2).toInt() * 1000 + reg.cap(3).toInt();
        } catch ( ... ) {
            return 0;
        }

        return 0;
    }

    void NodeIPC::getSyncing() {
        if ( !queueRequest(NodeRequest(NonVisual, GetSyncing, "eth_syncing")) ) {
            return bail();
        }
    }

    void NodeIPC::unlockAccount(const QString &hash, const QString &password, int duration, int index)
    {
        Q_UNUSED(duration); // lets just default here, hopefully geth does what parity now
        QJsonArray params;
        params.append(hash);
        params.append(password);

        // params.append(Helpers::toHexStr(duration));

        if ( !queueRequest(NodeRequest(UnlockAccount, "personal_unlockAccount", params, index)) ) {
            return bail();
        }
    }

    void NodeIPC::getFilterChanges(const QString& filterID, const QString& internalFilterID) {
        if ( filterID < 0 ) {
            setError("Filter ID invalid");
            return bail();
        }

        QJsonArray params;
        params.append(filterID);

        NodeRequest request(NonVisual, GetFilterChanges, "eth_getFilterChanges", params);
        QVariantMap userData;
        userData["internalFilterID"] = internalFilterID;
        request.setUserData(userData);

        if ( !queueRequest(request) ) {
            return bail();
        }
    }

    void NodeIPC::handleGetFilterChanges() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        QJsonArray ar = jv.toArray();

        foreach( const QJsonValue v, ar ) {
            if ( v.isObject() ) { // event filter result
                const QJsonObject logs = v.toObject();
                const QString internalFilterID = fActiveRequest.getUserData().value("internalFilterID").toString();
                emit newEvent(logs, fActiveRequest.getType() == GetFilterChanges, internalFilterID); // get logs is not "new"
            } else { // block filter (we don't use transaction filters yet)
                const QString hash = v.toString("bogus");
                getBlockByHash(hash);
            }
        }

        done();
    }

    void NodeIPC::uninstallFilter(const QString& internalID) {
        if ( internalID.isEmpty() ) {
            setError("internalID not set");
            return bail(true);
        }

        bool isEventFilter = fEventFilterIDs.contains(internalID);
        const QString filterID = isEventFilter ? fEventFilterIDs.value(internalID) : internalID; // block filter is direct

        QJsonArray params;
        params.append(filterID);

        NodeRequest request(UninstallFilter, "eth_uninstallFilter", params);
        if ( isEventFilter ) {
            QVariantMap userData;
            userData["internalID"] = internalID;
            request.setUserData(userData);
        }

        if ( !queueRequest(request) ) {
            return bail();
        }
    }

    void NodeIPC::getLogs(const QStringList& addresses, const QJsonArray& topics, quint64 fromBlock, const QString& internalID) {
        QJsonArray params;
        QJsonObject o;
        o["fromBlock"] = fromBlock == 0 ? "latest" : Helpers::toHexStr(fromBlock);
        o["address"] = QJsonArray::fromStringList(addresses);
        if ( topics.size() > 0 && !topics.at(0).toString().isEmpty() ) {
            o["topics"] = topics;
        }
        params.append(o);

        // we can use getFilterChanges as result is the same
        NodeRequest request(NonVisual, GetLogs, "eth_getLogs", params);
        QVariantMap userData;
        userData["internalFilterID"] = internalID;
        request.setUserData(userData);
        if ( !queueRequest(request) ) {
            return bail();
        }
    }

    bool NodeIPC::isThinClient() const
    {
        return false;
    }

    void NodeIPC::handleUninstallFilter() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        if ( fActiveRequest.getUserData().contains("internalID") ) {
            const QString internalID = fActiveRequest.getUserData().value("internalID").toString();
            fEventFilterIDs.remove(internalID);
            // qDebug() << "removed event filter: " << internalID << "\n";
        }

        done();
    }

    void NodeIPC::getClientVersion() {
        if ( !queueRequest(NodeRequest(NonVisual, GetClientVersion, "web3_clientVersion")) ) {
            return bail();
        }
    }

    void NodeIPC::getNetVersion() {
        if ( !queueRequest(NodeRequest(NonVisual, GetNetVersion, "net_version")) ) {
            return bail();
        }
    }

    bool NodeIPC::getSyncingVal() const {
        return fSyncing;
    }

    quint64 NodeIPC::getCurrentBlock() const {
        return fCurrentBlock;
    }

    quint64 NodeIPC::getHighestBlock() const {
        return fHighestBlock;
    }

    quint64 NodeIPC::getStartingBlock() const {
        return fStartingBlock;
    }

    void NodeIPC::getTransactionByHash(const QString& hash) {
        QJsonArray params;
        params.append(hash);

        if ( !queueRequest(NodeRequest(NonVisual, GetTransactionByHash, "eth_getTransactionByHash", params)) ) {
            return bail();
        }
    }

    void NodeIPC::handleGetTransactionByHash() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        emit newTransaction(jv.toObject());
        done();
    }

    void NodeIPC::getBlockByHash(const QString& hash) {
        QJsonArray params;
        params.append(hash);
        params.append(true); // get transaction bodies

        if ( !queueRequest(NodeRequest(NonVisual ,GetBlock, "eth_getBlockByHash", params)) ) {
            return bail();
        }
    }

    void NodeIPC::getBlockByNumber(quint64 blockNum) {
        QJsonArray params;
        params.append(Helpers::toHexStr(blockNum));
        params.append(true); // get transaction bodies

        if ( !queueRequest(NodeRequest(NonVisual ,GetBlock, "eth_getBlockByNumber", params)) ) {
            return bail();
        }
    }

    void NodeIPC::handleGetBlock() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        const QJsonObject block = jv.toObject();
        const quint64 num = Helpers::toQUInt64(block.value("number"));
        emit getBlockNumberDone(num);
        emit newBlock(block);
        done();
    }


    void NodeIPC::getTransactionReceipt(const QString& hash) {
        QJsonArray params;
        params.append(hash);

        if ( !queueRequest(NodeRequest(NonVisual, GetTransactionReceipt, "eth_getTransactionReceipt", params)) ) {
            return bail();
        }
    }

    const QString NodeIPC::defaultIPCPath(const QString& dataDir, bool testnet) {
#ifdef Q_OS_WIN32
        Q_UNUSED(dataDir);
        Q_UNUSED(testnet);
        return "\\\\.\\pipe\\geth.ipc";
#else
        const QString mid_fix = testnet ? "/rinkeby" : "";
        return QDir::cleanPath(dataDir + mid_fix + "/geth.ipc");
#endif
    }

    const QString NodeIPC::defaultGethPath() {
#ifdef Q_OS_WIN32
        return QApplication::applicationDirPath() + "/geth.exe";
#else
#ifdef Q_OS_MACX
        return QApplication::applicationDirPath() + "/geth";
#else
        return "/usr/bin/geth";
#endif
#endif
    }

    void NodeIPC::handleGetTransactionReceipt() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }


        emit getTransactionReceiptDone(jv.toObject());
        done();
    }

    void NodeIPC::handleGetClientVersion() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        fClientVersion = jv.toString();

        const int vn = parseVersionNum();
        if ( vn > 0 && vn < 104019 ) {
            setError("Geth version 1.4.18 and older are not ready for the upcoming 4th hard fork. Please update Geth to ensure you are ready.");
            emit error();
        }

        if ( vn > 0 && vn < 107002 ) {
            setError("Geth version older than 1.7.2 is no longer supported. Please upgrade geth to 1.7.2+.");
            emit error();
        }

        emit clientVersionChanged(fClientVersion);
        done();
    }

    void NodeIPC::handleGetNetVersion() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        bool ok = false;
        fNetVersion = jv.toString().toInt(&ok);

        if ( !ok ) {
            setError("Unable to parse net version string: " + jv.toString());
            return bail(true);
        }

        emit netVersionChanged(fNetVersion);
        emit ipcReady();
        done();
    }

    void NodeIPC::handleGetSyncing() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        if ( jv.isNull() || ( jv.isBool() && !jv.toBool(false) ) ) {
            if ( fSyncing ) {
                fSyncing = false;
                if ( fBlockFilterID.isEmpty() ) {
                    newBlockFilter();
                }
                emit syncingChanged(fSyncing);
            }

            return done();
        }

        const QJsonObject syncing = jv.toObject();
        fCurrentBlock = Helpers::toQUInt64(syncing.value("currentBlock"));
        fHighestBlock = Helpers::toQUInt64(syncing.value("highestBlock"));
        fStartingBlock = Helpers::toQUInt64(syncing.value("startingBlock"));

        if ( !fSyncing ) {
            if ( !fBlockFilterID.isEmpty() ) {
                uninstallFilter(fBlockFilterID);
                fBlockFilterID.clear();
            }
            fSyncing = true;
        }

        emit syncingChanged(fSyncing);
        done();
    }

    void NodeIPC::handleUnlockAccount()
    {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            // special case, we def. need to remove all subrequests, but not stop timer
            fRequestQueue.clear();
            return bail(true);
        }

        const bool result = jv.toBool(false);

        if ( !result ) {
            setError("Unlock account failure");
            emit error();
        }

        emit unlockAccountDone(result, fActiveRequest.getIndex());
        done();
    }

    void NodeIPC::bail(bool soft) {
        EtherLog::logMsg("bail[" + (soft ? QString("soft") : QString("hard")) + "]: " + fError, LS_Error);

        if ( !soft ) {
            emit stopTimer();
            fRequestQueue.clear();
        }

        fActiveRequest = NodeRequest(None);
        errorOut();
    }

    void NodeIPC::setError(const QString& error) {
        fError = error;
        EtherLog::logMsg(error, LS_Error);
    }

    void NodeIPC::errorOut() {
        emit error();
        emit connectionStateChanged();
        done();
    }

    void NodeIPC::done() {
        emit requestDone();
    }

    QJsonObject NodeIPC::methodToJSON(const NodeRequest& request) {
        QJsonObject result;

        result.insert("jsonrpc", QJsonValue(QString("2.0")));
        result.insert("method", QJsonValue(request.getMethod()));
        result.insert("id", QJsonValue(request.getCallID()));
        result.insert("params", QJsonValue(request.getParams()));

        return result;
    }

    bool NodeIPC::queueRequest(const NodeRequest& request) {
        if ( fActiveRequest.burden() == None ) {
            emit requestChanged();
            return writeRequest(request);
        } else {
            fRequestQueue.enqueue(request);
            return true;
        }
    }

    bool NodeIPC::writeRequest(const NodeRequest& request) {
        fActiveRequest = request;
        if ( fActiveRequest.burden() == Full ) { // only update to busy if we're not doing background tasks
            emit busyChanged(getBusy());
        }

        QJsonDocument doc(methodToJSON(fActiveRequest));
        const QString msg(doc.toJson(QJsonDocument::Compact));

        if ( !endpointWritable() ) {
            setError("Socket not writeable");
            fCode = 0;
            return false;
        }

        const QByteArray sendBuf = msg.toUtf8();
        EtherLog::logMsg("Sent: " + msg, LS_Debug);
        const int sent = endpointWrite(sendBuf);

        if ( sent <= 0 ) {
            setError("Error on socket write: " + fSocket.errorString());
            fCode = 0;
            return false;
        }

        return true;
    }

    bool NodeIPC::readData() {
        fReadBuffer += QString(endpointRead()).trimmed();

        if ( fReadBuffer.at(0) == '{' && fReadBuffer.at(fReadBuffer.length() - 1) == '}' && fReadBuffer.count('{') == fReadBuffer.count('}') ) {
            fReceivedMsg =  fReadBuffer;
            EtherLog::logMsg("Received: " + fReadBuffer, LS_Debug);
            fReadBuffer.clear();
            return true;
        }

        return false;
    }

    bool NodeIPC::readReply(QJsonValue& result) {
        if ( fReceivedMsg.isEmpty() ) {
            setError("Error on socket read: " + fSocket.errorString());
            fCode = 0;
            return false;
        }

        QJsonParseError parseError;
        QJsonDocument resDoc = QJsonDocument::fromJson(fReceivedMsg.toUtf8(), &parseError);

        if ( parseError.error != QJsonParseError::NoError ) {
            qDebug() << fReceivedMsg << "\n";
            setError("Response parse error: " + parseError.errorString());
            fCode = 0;
            return false;
        }

        const QJsonObject obj = resDoc.object();
        const int objID = obj["id"].toInt(-1);

        if ( objID != fActiveRequest.getCallID() ) { // TODO
            setError("Call number mismatch " + QString::number(objID) + " != " + QString::number(fActiveRequest.getCallID()));
            fCode = 0;
            return false;
        }

        result = obj["result"];

        // get filter changes bugged, returns null on result array, see https://github.com/ethereum/go-ethereum/issues/2746
        if ( result.isNull() && (fActiveRequest.getType() == GetFilterChanges || fActiveRequest.getType() == GetAccountRefs) ) {
            result = QJsonValue(QJsonArray());
        }

        if ( result.isUndefined() || result.isNull() ) {
            if ( obj.contains("error") ) {
                if ( obj["error"].toObject().contains("message") ) {
                    fError = obj["error"].toObject()["message"].toString();
                }

                if ( obj["error"].toObject().contains("code") ) {
                    fCode = obj["error"].toObject()["code"].toInt();
                }

                return false;
            }

            if ( fActiveRequest.getType() != GetTransactionByHash ) { // this can happen if out of sync, it's not fatal for transaction get
                setError("Result object undefined in IPC response for request: " + fActiveRequest.getMethod());
                qDebug() << fReceivedMsg << "\n";
                return false;
            }
        }

        return true;
    }

    bool NodeIPC::readVin(BigInt::Vin& result) {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return false;
        }

        std::string hexStr = jv.toString("0x0").remove(0, 2).toStdString();
        result = BigInt::Vin(hexStr, 16);

        return true;
    }

    bool NodeIPC::readNumber(quint64& result) {
        BigInt::Vin r;
        if ( !readVin(r) ) {
            return false;
        }

        result = r.toUlong();
        return true;
    }

    const QString NodeIPC::getActiveRequestName() const
    {
        return fActiveRequest.getMethod();
    }

    void NodeIPC::handleRequest()
    {
        switch ( fActiveRequest.getType() ) {
        case NoRequest: {
            break;
        }
        case NewAccount: {
                handleNewAccount();
                break;
            }
        case UnlockAccount: {
                handleUnlockAccount();
                break;
            }
        case GetBlockNumber: {
                handleGetBlockNumber();
                break;
            }
        case GetAccountRefs: {
                handleGetAccounts();
                break;
            }
        case GetBalance: {
                handleAccountBalance();
                break;
            }
        case GetTransactionCount: {
                handleAccountTransactionCount();
                break;
            }
        case GetPeerCount: {
                handleGetPeerCount();
                break;
            }
        case SendTransaction: {
                handleSendTransaction();
                break;
            }
        case SignTransaction: {
                handleSignTransaction();
                break;
            }
        case Call: {
                handleCall();
                break;
            }
        case SendRawTransaction: {
                handleSendTransaction();
                break;
            }
        case GetGasPrice: {
                handleGetGasPrice();
                break;
            }
        case EstimateGas: {
                handleEstimateGas();
                break;
            }
        case NewBlockFilter: {
                handleNewBlockFilter();
                break;
            }
        case NewEventFilter: {
                handleNewEventFilter();
                break;
            }
        case GetFilterChanges: {
                handleGetFilterChanges();
                break;
            }
        case UninstallFilter: {
                handleUninstallFilter();
                break;
            }
        case GetTransactionByHash: {
                handleGetTransactionByHash();
                break;
            }
        case GetBlock: {
                handleGetBlock();
                break;
            }
        case GetClientVersion: {
                handleGetClientVersion();
                break;
            }
        case GetNetVersion: {
                handleGetNetVersion();
                break;
            }
        case GetSyncing: {
                handleGetSyncing();
                break;
            }
        case GetLogs: {
                handleGetFilterChanges();
                break;
            }
        case GetTransactionReceipt: {
                handleGetTransactionReceipt();
                break;
            }
        }
    }

    void NodeIPC::onSocketError(QLocalSocket::LocalSocketError err) {
        fError = fSocket.errorString();
        fCode = err;
    }

    void NodeIPC::onSocketReadyRead() {
        if ( !getBusy() ) {
            return; // probably error-ed out
        }

        if ( !readData() ) {
            return; // not finished yet
        }

        // safe because we never handle more than 1 request at a time!
        QtConcurrent::run(QThreadPool::globalInstance(), this, &NodeIPC::handleRequest);
    }

}
