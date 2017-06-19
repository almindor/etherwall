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

#include "etheripc.h"
#include "helpers.h"
#include <QSettings>
#include <QFileInfo>

// windblows hacks coz windblows sucks
#ifdef Q_OS_WIN32
    #include <windows.h>
#endif

namespace Etherwall {

// *************************** RequestIPC **************************** //
    int RequestIPC::sCallID = 0;

    RequestIPC::RequestIPC(RequestBurden burden, RequestTypes type, const QString method, const QJsonArray params, int index) :
        fCallID(sCallID++), fType(type), fMethod(method), fParams(params), fIndex(index), fBurden(burden)
    {
    }

    RequestIPC::RequestIPC(RequestTypes type, const QString method, const QJsonArray params, int index) :
        fCallID(sCallID++), fType(type), fMethod(method), fParams(params), fIndex(index), fBurden(Full)
    {
    }

    RequestIPC::RequestIPC(RequestBurden burden) : fBurden(burden)
    {
    }

    RequestTypes RequestIPC::getType() const {
        return fType;
    }

    const QString& RequestIPC::getMethod() const {
        return fMethod;
    }

    const QJsonArray& RequestIPC::getParams() const {
        return fParams;
    }

    int RequestIPC::getIndex() const {
        return fIndex;
    }

    int RequestIPC::getCallID() const {
        return fCallID;
    }

    RequestBurden RequestIPC::burden() const {
        return fBurden;
    }

// *************************** EtherIPC **************************** //

    EtherIPC::EtherIPC(GethLog& gethLog) :
        fPath(), fBlockFilterID(), fClosingApp(false), fPeerCount(0), fActiveRequest(None),
        fGeth(), fStarting(0), fGethLog(gethLog),
        fSyncing(false), fCurrentBlock(0), fHighestBlock(0), fStartingBlock(0),
        fConnectAttempts(0), fKillTime(), fExternal(false), fEventFilterID()
    {
        connect(&fSocket, (void (QLocalSocket::*)(QLocalSocket::LocalSocketError))&QLocalSocket::error, this, &EtherIPC::onSocketError);
        connect(&fSocket, &QLocalSocket::readyRead, this, &EtherIPC::onSocketReadyRead);
        connect(&fSocket, &QLocalSocket::connected, this, &EtherIPC::connectedToServer);
        connect(&fSocket, &QLocalSocket::disconnected, this, &EtherIPC::disconnectedFromServer);
        connect(&fGeth, &QProcess::started, this, &EtherIPC::connectToServer);

        const QSettings settings;

        connect(&fTimer, &QTimer::timeout, this, &EtherIPC::onTimer);
    }

    EtherIPC::~EtherIPC() {
        fGeth.kill();
    }

    void EtherIPC::init() {
        QStringList args = buildGethArgs(); // has to run first

        fConnectAttempts = 0;
        if ( fStarting <= 0 ) { // try to connect without starting geth
            EtherLog::logMsg("Etherwall starting", LS_Info);
            fStarting = 1;
            emit startingChanged(fStarting);
            return connectToServer();
        }

        QSettings settings;

        const QString progStr = settings.value("geth/path", DefaultGethPath()).toString();

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

    void EtherIPC::waitConnect() {
        if ( fStarting == 1 ) {
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

    void EtherIPC::connectToServer() {
        fActiveRequest = RequestIPC(Full);
        emit busyChanged(getBusy());
        if ( fSocket.state() != QLocalSocket::UnconnectedState ) {
            setError("Already connected");
            return bail(true);
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

    void EtherIPC::connectedToServer() {
        done();

        getClientVersion();
        getBlockNumber();
        newBlockFilter();
        getSyncing();
        getNetVersion();

        if ( fStarting == 1 ) {
            fExternal = true;
            emit externalChanged(true);
            fGethLog.append("Attached to external geth, see logs in terminal window.");
        }
        fStarting = 3;
        EtherLog::logMsg("Connected to IPC socket");
    }

    void EtherIPC::connectionTimeout() {
        if ( fSocket.state() != QLocalSocket::ConnectedState ) {
            fSocket.abort();
            fStarting = -1;
            emit startingChanged(fStarting);
            setError("Unable to establish IPC connection to Geth. Fix path to Geth and try again.");
            bail();
        }
    }

    bool EtherIPC::getBusy() const {
        return (fActiveRequest.burden() != None);
    }

    bool EtherIPC::getExternal() const {
        return fExternal;
    }

    bool EtherIPC::getStarting() const {
        return (fStarting == 1 || fStarting == 2);
    }

    bool EtherIPC::getClosing() const {
        return fClosingApp;
    }

    const QString& EtherIPC::getError() const {
        return fError;
    }

    int EtherIPC::getCode() const {
        return fCode;
    }


    void EtherIPC::setInterval(int interval) {
        QSettings settings;
        settings.setValue("ipc/interval", (int) (interval / 1000));
        fTimer.setInterval(interval);
    }

    bool EtherIPC::getTestnet() const {
        return fNetVersion == 4;
    }

    const QString EtherIPC::getNetworkPostfix() const {
        return Helpers::networkPostfix(fNetVersion);
    }

    bool EtherIPC::killGeth() {
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

    bool EtherIPC::closeApp() {
        EtherLog::logMsg("Closing etherwall");
        fClosingApp = true;
        fTimer.stop();
        emit closingChanged(true);

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

            if ( !fEventFilterID.isEmpty() ) { // remove event filter if still connected
                uninstallFilter(fEventFilterID);
                fEventFilterID.clear();
                removed = true;
            }

            if ( removed ) {
                return false;
            }
        }

        if ( fSocket.state() != QLocalSocket::UnconnectedState ) { // wait for clean disconnect
            fActiveRequest = RequestIPC(Full);
            fSocket.disconnectFromServer();
            return false;
        }

        return killGeth();
    }

    void EtherIPC::registerEventFilters(const QStringList& addresses, const QStringList& topics) {
        if ( !fEventFilterID.isEmpty() ) {
            uninstallFilter(fEventFilterID);
            fEventFilterID.clear();
        }

        if ( addresses.length() > 0 ) {
            newEventFilter(addresses, topics);
        }
    }

    void EtherIPC::loadLogs(const QStringList& addresses, const QStringList& topics, quint64 fromBlock) {
        if ( addresses.length() > 0 ) {
            getLogs(addresses, topics, fromBlock);
        }
    }

    void EtherIPC::disconnectedFromServer() {
        if ( fClosingApp ) { // expected
            return;
        }

        fError = fSocket.errorString();
        bail();
    }

    void EtherIPC::getAccounts() {
        if ( !queueRequest(RequestIPC(GetAccountRefs, "personal_listAccounts", QJsonArray())) ) {
            return bail();
        }
    }

    void EtherIPC::handleGetAccounts() {
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

    bool EtherIPC::refreshAccount(const QString& hash, int index) {
        if ( getBalance(hash, index) ) {
            return getTransactionCount(hash, index);
        }

        return false;
    }

    bool EtherIPC::getBalance(const QString& hash, int index) {
        QJsonArray params;
        params.append(hash);
        params.append(QString("latest"));
        if ( !queueRequest(RequestIPC(GetBalance, "eth_getBalance", params, index)) ) {
            bail();
            return false;
        }

        return true;
    }

    bool EtherIPC::getTransactionCount(const QString& hash, int index) {
        QJsonArray params;
        params.append(hash);
        params.append(QString("latest"));
        if ( !queueRequest(RequestIPC(GetTransactionCount, "eth_getTransactionCount", params, index)) ) {
            bail();
            return false;
        }

        return true;
    }


    void EtherIPC::handleAccountBalance() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        const QString decStr = Helpers::toDecStrEther(jv);
        const int index = fActiveRequest.getIndex();

        emit accountBalanceChanged(index, decStr);
        done();
    }

    void EtherIPC::handleAccountTransactionCount() {
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

    void EtherIPC::newAccount(const QString& password, int index) {
        QJsonArray params;
        params.append(password);
        if ( !queueRequest(RequestIPC(NewAccount, "personal_newAccount", params, index)) ) {
            return bail();
        }
    }

    void EtherIPC::handleNewAccount() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        const QString result = jv.toString();
        emit newAccountDone(result, fActiveRequest.getIndex());
        done();
    }

    void EtherIPC::getBlockNumber() {
        if ( !queueRequest(RequestIPC(NonVisual, GetBlockNumber, "eth_blockNumber")) ) {
            return bail();
        }
    }

    void EtherIPC::handleGetBlockNumber() {
        quint64 result;
        if ( !readNumber(result) ) {
             return bail();
        }

        fBlockNumber = result;
        emit getBlockNumberDone(result);
        done();
    }

    quint64 EtherIPC::blockNumber() const {
        return fBlockNumber;
    }

    int EtherIPC::network() const
    {
        return fNetVersion;
    }

    quint64 EtherIPC::nonceStart() const
    {
        switch ( fNetVersion ) {
            case 2: return 0x0100000;
            case 3: return 0x0100000;
            case 4: return 0;
        }

        return 0;
    }

    void EtherIPC::getPeerCount() {
        if ( !queueRequest(RequestIPC(NonVisual, GetPeerCount, "net_peerCount")) ) {
            return bail();
        }
    }

    void EtherIPC::handleGetPeerCount() {
        if ( !readNumber(fPeerCount) ) {
             return bail();
        }

        emit peerCountChanged(fPeerCount);
        done();
    }

    void EtherIPC::sendTransaction(const Ethereum::Tx& tx, const QString& password) {
        QJsonArray params;
        QJsonObject p;
        p["from"] = tx.fromStr();
        p["value"] = tx.valueHex();
        if ( !tx.isContractDeploy() ) {
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

        if ( !queueRequest(RequestIPC(SendTransaction, "personal_signAndSendTransaction", params)) ) {
            return bail(true); // softbail
        }
    }

    void EtherIPC::signTransaction(const Ethereum::Tx &tx, const QString &password)
    {
        qDebug() << "called sendTX with manual signing\n";
        unlockAccount(tx.fromStr(), password, 5, 0);
        signTransaction(tx);
    }

    void EtherIPC::signTransaction(const Ethereum::Tx &tx)
    {
        QJsonArray params;
        QJsonObject p;
        p["from"] = tx.fromStr();
        p["value"] = tx.valueHex();
        if ( !tx.isContractDeploy() ) {
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

        if ( !queueRequest(RequestIPC(SignTransaction, "eth_signTransaction", params)) ) {
            return bail(true); // softbail
        }
    }

    void EtherIPC::sendRawTransaction(const Ethereum::Tx &tx)
    {
        sendRawTransaction(Helpers::hexPrefix(tx.encodeRLP(true)));
    }

    void EtherIPC::sendRawTransaction(const QString &rlp)
    {
        QJsonArray params;
        params.append(rlp);

        if ( !queueRequest(RequestIPC(SendRawTransaction, "eth_sendRawTransaction", params)) ) {
            return bail(true); // softbail
        }
    }

    void EtherIPC::handleSendTransaction() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail(true); // softbail
        }

        const QString hash = jv.toString();
        emit sendTransactionDone(hash);
        done();
    }

    void EtherIPC::handleSignTransaction()
    {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail(true); // softbail
        }

        const QString hash = jv.toObject().value("raw").toString();
        emit signTransactionDone(hash);
        done();
    }

    int EtherIPC::getConnectionState() const {
        if ( fSocket.state() == QLocalSocket::ConnectedState ) {
            return 1; // TODO: add higher states per peer count!
        }

        return 0;
    }

    void EtherIPC::getGasPrice() {
        if ( !queueRequest(RequestIPC(GetGasPrice, "eth_gasPrice")) ) {
            return bail();
        }
    }

    void EtherIPC::handleGetGasPrice() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        const QString decStr = Helpers::toDecStrEther(jv);

        emit getGasPriceDone(decStr);
        done();
    }

    quint64 EtherIPC::peerCount() const {
        return fPeerCount;
    }

    void EtherIPC::ipcReady()
    {
        const QSettings settings;
        setInterval(settings.value("ipc/interval", 10).toInt() * 1000); // re-set here, used for inheritance purposes
        fTimer.start(); // should happen after filter creation, might need to move into last filter response handler
        // if we connected to external geth, put that info in geth log
        emit startingChanged(fStarting);
        emit connectToServerDone();
        emit connectionStateChanged();
    }

    bool EtherIPC::endpointWritable()
    {
        return fSocket.isWritable();
    }

    qint64 EtherIPC::endpointWrite(const QByteArray &data)
    {
        return fSocket.write(data);
    }

    const QByteArray EtherIPC::endpointRead()
    {
        return fSocket.readAll();
    }

    const QStringList EtherIPC::buildGethArgs()
    {
        QSettings settings;
        QString argStr = settings.value("geth/args", DefaultGethArgs).toString();
        const QString ddStr = settings.value("geth/datadir", DefaultDataDir).toString();

        // check deprecated options and replace them
        if ( argStr.contains("--light") || argStr.contains("--fast") ) {
            argStr = argStr.replace("--light", "--syncmode=light");
            argStr = argStr.replace("--fast", "--syncmode=fast");
            settings.setValue("geth/args", argStr);
            qDebug() << "replaced args\n";
        }

        QStringList args;
        args.append("--nousb");
        bool testnet = settings.value("geth/testnet", false).toBool();
        fPath = DefaultIPCPath(ddStr, testnet);
        if ( testnet ) { // geth 1.6.0+ only
            args = (argStr + " --datadir " + ddStr + "/rinkeby").split(' ', QString::SkipEmptyParts);
            args.append("--rinkeby");
        } else {
            args = (argStr + " --datadir " + ddStr).split(' ', QString::SkipEmptyParts);
        }

        return args;
    }

    void EtherIPC::estimateGas(const QString& from, const QString& to, const QString& valStr,
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
        if ( !queueRequest(RequestIPC(EstimateGas, "eth_estimateGas", params)) ) {
            return bail();
        }
    }

    void EtherIPC::handleEstimateGas() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail(true); // probably gas too low
        }

        const QString price = Helpers::toDecStr(jv);

        emit estimateGasDone(price);

        done();
    }

    void EtherIPC::newBlockFilter() {
        if ( !fBlockFilterID.isEmpty() ) {
            setError("Filter already set");
            return bail(true);
        }

        if ( !queueRequest(RequestIPC(NewBlockFilter, "eth_newBlockFilter")) ) {
            return bail();
        }
    }

    void EtherIPC::newEventFilter(const QStringList& addresses, const QStringList& topics) {
        QJsonArray params;
        QJsonObject o;
        o["address"] = QJsonArray::fromStringList(addresses);
        if ( topics.length() > 0 && topics.at(0).length() > 0 ) {
            o["topics"] = parseTopics(topics);
        }
        params.append(o);

        if ( !queueRequest(RequestIPC(NewEventFilter, "eth_newFilter", params)) ) {
            return bail();
        }
    }

    void EtherIPC::handleNewBlockFilter() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }
        fBlockFilterID = jv.toString();

        if ( fBlockFilterID.isEmpty() ) {
            setError("Block filter ID invalid");
            return bail();
        }

        done();
    }

    void EtherIPC::handleNewEventFilter() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }
        fEventFilterID = jv.toString();

        if ( fEventFilterID.isEmpty() ) {
            setError("Event filter ID invalid");
            return bail();
        }

        done();
    }

    void EtherIPC::onTimer() {
        getPeerCount();
        getSyncing();

        if ( !fBlockFilterID.isEmpty() && !fSyncing ) {
            getFilterChanges(fBlockFilterID);
        } else {
            getBlockNumber();
        }

        if ( !fEventFilterID.isEmpty() ) {
            getFilterChanges(fEventFilterID);
        }
    }

    int EtherIPC::parseVersionNum() const {
        QRegExp reg("^Geth/v([0-9]+)\\.([0-9]+)\\.([0-9]+).*$");
        reg.indexIn(fClientVersion);
        if ( reg.captureCount() == 3 ) try { // it's geth
            return reg.cap(1).toInt() * 100000 + reg.cap(2).toInt() * 1000 + reg.cap(3).toInt();
        } catch ( ... ) {
            return 0;
        }

        return 0;
    }

    const QJsonArray EtherIPC::parseTopics(const QStringList &topics)
    {
        QJsonArray result;
        foreach (const QString& topic, topics) {
            if ( topic == "null" ) {
                result.append(QJsonValue());
            } else {
                result.append(topic);
            }
        }

        return result;
    }

    void EtherIPC::getSyncing() {
        if ( !queueRequest(RequestIPC(NonVisual, GetSyncing, "eth_syncing")) ) {
            return bail();
        }
    }

    void EtherIPC::unlockAccount(const QString &hash, const QString &password, int duration, int index)
    {
        Q_UNUSED(duration); // lets just default here, hopefully geth does what parity now
        QJsonArray params;
        params.append(hash);
        params.append(password);

        // params.append(Helpers::toHexStr(duration));

        if ( !queueRequest(RequestIPC(UnlockAccount, "personal_unlockAccount", params, index)) ) {
            return bail();
        }
    }

    void EtherIPC::getFilterChanges(const QString& filterID) {
        if ( filterID < 0 ) {
            setError("Filter ID invalid");
            return bail();
        }

        QJsonArray params;
        params.append(filterID);

        if ( !queueRequest(RequestIPC(NonVisual, GetFilterChanges, "eth_getFilterChanges", params)) ) {
            return bail();
        }
    }

    void EtherIPC::handleGetFilterChanges() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        QJsonArray ar = jv.toArray();

        foreach( const QJsonValue v, ar ) {
            if ( v.isObject() ) { // event filter result
                const QJsonObject logs = v.toObject();
                emit newEvent(logs, fActiveRequest.getType() == GetFilterChanges); // get logs is not "new"
            } else { // block filter (we don't use transaction filters yet)
                const QString hash = v.toString("bogus");
                getBlockByHash(hash);
            }
        }

        done();
    }

    void EtherIPC::uninstallFilter(const QString& filter) {
        if ( filter.isEmpty() ) {
            setError("Filter not set");
            return bail(true);
        }

        //qDebug() << "uninstalling filter: " << fBlockFilterID << "\n";

        QJsonArray params;
        params.append(filter);

        if ( !queueRequest(RequestIPC(UninstallFilter, "eth_uninstallFilter", params)) ) {
            return bail();
        }
    }

    void EtherIPC::getLogs(const QStringList& addresses, const QStringList& topics, quint64 fromBlock) {
        QJsonArray params;
        QJsonObject o;
        o["fromBlock"] = fromBlock == 0 ? "latest" : Helpers::toHexStr(fromBlock);
        o["address"] = QJsonArray::fromStringList(addresses);
        if ( topics.length() > 0 && topics.at(0).length() > 0 ) {
            o["topics"] = parseTopics(topics);
        }
        params.append(o);

        // we can use getFilterChanges as result is the same
        if ( !queueRequest(RequestIPC(GetLogs, "eth_getLogs", params)) ) {
            return bail();
        }
    }

    bool EtherIPC::isThinClient() const
    {
        return false;
    }

    void EtherIPC::handleUninstallFilter() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        done();
    }

    void EtherIPC::getClientVersion() {
        if ( !queueRequest(RequestIPC(NonVisual, GetClientVersion, "web3_clientVersion")) ) {
            return bail();
        }
    }

    void EtherIPC::getNetVersion() {
        if ( !queueRequest(RequestIPC(NonVisual, GetNetVersion, "net_version")) ) {
            return bail();
        }
    }

    bool EtherIPC::getSyncingVal() const {
        return fSyncing;
    }

    quint64 EtherIPC::getCurrentBlock() const {
        return fCurrentBlock;
    }

    quint64 EtherIPC::getHighestBlock() const {
        return fHighestBlock;
    }

    quint64 EtherIPC::getStartingBlock() const {
        return fStartingBlock;
    }

    void EtherIPC::getTransactionByHash(const QString& hash) {
        QJsonArray params;
        params.append(hash);

        if ( !queueRequest(RequestIPC(GetTransactionByHash, "eth_getTransactionByHash", params)) ) {
            return bail();
        }
    }

    void EtherIPC::handleGetTransactionByHash() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        emit newTransaction(TransactionInfo(jv.toObject()));
        done();
    }

    void EtherIPC::getBlockByHash(const QString& hash) {
        QJsonArray params;
        params.append(hash);
        params.append(true); // get transaction bodies

        if ( !queueRequest(RequestIPC(GetBlock, "eth_getBlockByHash", params)) ) {
            return bail();
        }
    }

    void EtherIPC::getBlockByNumber(quint64 blockNum) {
        QJsonArray params;
        params.append(Helpers::toHexStr(blockNum));
        params.append(true); // get transaction bodies

        if ( !queueRequest(RequestIPC(GetBlock, "eth_getBlockByNumber", params)) ) {
            return bail();
        }
    }

    void EtherIPC::handleGetBlock() {
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

    void EtherIPC::getTransactionReceipt(const QString& hash) {
        QJsonArray params;
        params.append(hash);

        if ( !queueRequest(RequestIPC(GetTransactionReceipt, "eth_getTransactionReceipt", params)) ) {
            return bail();
        }
    }

    void EtherIPC::handleGetTransactionReceipt() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }


        emit getTransactionReceiptDone(jv.toObject());
        done();
    }

    void EtherIPC::handleGetClientVersion() {
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

        if ( vn > 0 && vn < 106000 ) {
            setError("Geth version older than 1.6.0 is no longer supported. Please upgrade geth to 1.6.0+.");
            emit error();
        }

        emit clientVersionChanged(fClientVersion);
        done();
    }

    void EtherIPC::handleGetNetVersion() {
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
        done();

        ipcReady();
    }

    void EtherIPC::handleGetSyncing() {
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

    void EtherIPC::handleUnlockAccount()
    {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            // special case, we def. need to remove all subrequests, but not stop timer
            fRequestQueue.clear();
            return bail(true);
        }

        const bool result = jv.toBool(false);
        qDebug() << "unlock account: " << result << "\n";

        if ( !result ) {
            setError("Unlock account failure");
            emit error();
        }

        emit unlockAccountDone(result, fActiveRequest.getIndex());
        done();
    }

    void EtherIPC::bail(bool soft) {
        qDebug() << "BAIL[" << soft << "]: " << fError << "\n";

        if ( !soft ) {
            fTimer.stop();
            fRequestQueue.clear();
        }

        fActiveRequest = RequestIPC(None);
        errorOut();
    }

    void EtherIPC::setError(const QString& error) {
        fError = error;
        EtherLog::logMsg(error, LS_Error);
    }

    void EtherIPC::errorOut() {
        emit error();
        emit connectionStateChanged();
        done();
    }

    void EtherIPC::done() {
        fActiveRequest = RequestIPC(None);
        if ( !fRequestQueue.isEmpty() ) {
            const RequestIPC request = fRequestQueue.dequeue();
            writeRequest(request);
        } else {
            emit busyChanged(getBusy());
        }
    }

    QJsonObject EtherIPC::methodToJSON(const RequestIPC& request) {
        QJsonObject result;

        result.insert("jsonrpc", QJsonValue(QString("2.0")));
        result.insert("method", QJsonValue(request.getMethod()));
        result.insert("id", QJsonValue(request.getCallID()));
        result.insert("params", QJsonValue(request.getParams()));

        return result;
    }

    bool EtherIPC::queueRequest(const RequestIPC& request) {
        if ( fActiveRequest.burden() == None ) {
            return writeRequest(request);
        } else {
            fRequestQueue.enqueue(request);
            return true;
        }
    }

    bool EtherIPC::writeRequest(const RequestIPC& request) {
        fActiveRequest = request;
        if ( fActiveRequest.burden() == Full ) {
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

    bool EtherIPC::readData() {
        fReadBuffer += QString(endpointRead()).trimmed();

        if ( fReadBuffer.at(0) == '{' && fReadBuffer.at(fReadBuffer.length() - 1) == '}' && fReadBuffer.count('{') == fReadBuffer.count('}') ) {
            EtherLog::logMsg("Received: " + fReadBuffer, LS_Debug);
            return true;
        }

        return false;
    }

    bool EtherIPC::readReply(QJsonValue& result) {
        const QString data = fReadBuffer;
        fReadBuffer.clear();

        if ( data.isEmpty() ) {
            setError("Error on socket read: " + fSocket.errorString());
            fCode = 0;
            return false;
        }

        QJsonParseError parseError;
        QJsonDocument resDoc = QJsonDocument::fromJson(data.toUtf8(), &parseError);

        if ( parseError.error != QJsonParseError::NoError ) {
            qDebug() << data << "\n";
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
                qDebug() << data << "\n";
                return false;
            }
        }

        return true;
    }

    bool EtherIPC::readVin(BigInt::Vin& result) {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return false;
        }

        std::string hexStr = jv.toString("0x0").remove(0, 2).toStdString();
        result = BigInt::Vin(hexStr, 16);

        return true;
    }

    bool EtherIPC::readNumber(quint64& result) {
        BigInt::Vin r;
        if ( !readVin(r) ) {
            return false;
        }

        result = r.toUlong();
        return true;
    }

    void EtherIPC::onSocketError(QLocalSocket::LocalSocketError err) {
        fError = fSocket.errorString();
        fCode = err;
    }

    void EtherIPC::onSocketReadyRead() {
        if ( !getBusy() ) {
            return; // probably error-ed out
        }

        if ( !readData() ) {
            return; // not finished yet
        }

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

}
