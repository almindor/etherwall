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
#include <QSettings>

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

    RequestIPC::RequestIPC() : fBurden(None)
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

    EtherIPC::EtherIPC() : fPendingTransactionsFilterID(-1), fBlockFilterID(-1), fClosingApp(false), fPeerCount(0)
    {
        connect(&fSocket, (void (QLocalSocket::*)(QLocalSocket::LocalSocketError))&QLocalSocket::error, this, &EtherIPC::onSocketError);
        connect(&fSocket, &QLocalSocket::readyRead, this, &EtherIPC::onSocketReadyRead);
        connect(&fSocket, &QLocalSocket::connected, this, &EtherIPC::connectedToServer);
        connect(&fSocket, &QLocalSocket::disconnected, this, &EtherIPC::disconnectedFromServer);

        const QSettings settings;

        fTimer.setInterval(settings.value("ipc/interval", 10).toInt() * 1000);
        connect(&fTimer, &QTimer::timeout, this, &EtherIPC::onTimer);
    }

    bool EtherIPC::getBusy() const {
        return (fActiveRequest.burden() != None);
    }

    const QString& EtherIPC::getError() const {
        return fError;
    }

    int EtherIPC::getCode() const {
        return fCode;
    }

    void EtherIPC::setInterval(int interval) {
        fTimer.setInterval(interval);
    }

    bool EtherIPC::closeApp() {
        fClosingApp = true;

        if ( fSocket.state() == QLocalSocket::UnconnectedState ) {
            return true;
        }

        if ( getBusy() ) { // wait for operation first
            return false;
        }

        if ( fSocket.state() == QLocalSocket::ConnectedState &&
             ( fPendingTransactionsFilterID >= 0 || fBlockFilterID >= 0) ) { // remove filters if still connected
            uninstallFilter(fPendingTransactionsFilterID);
            uninstallFilter(fBlockFilterID);
            return false;
        }

        if ( fSocket.state() != QLocalSocket::UnconnectedState ) { // wait for clean disconnect
            fSocket.disconnectFromServer();
            return false;
        }

        return true;
    }

    void EtherIPC::connectToServer(const QString& path) {
        fActiveRequest = RequestIPC(Full);
        emit busyChanged(getBusy());
        fPath = path;
        if ( fSocket.state() != QLocalSocket::UnconnectedState ) {
            fError = "Already connected";
            return bail();
        }

        fSocket.connectToServer(path);
        QTimer::singleShot(2000, this, SLOT(disconnectedFromServer()));
    }

    void EtherIPC::connectedToServer() {
        done();

        getBlockNumber(); // initial
        newPendingTransactionFilter();
        newBlockFilter();
        fTimer.start(); // should happen after filter creation, might need to move into last filter response handler

        emit connectToServerDone();
        emit connectionStateChanged();
    }

    void EtherIPC::disconnectedFromServer() {
        if ( fClosingApp ) { // expected
            return;
        }

        if ( fSocket.state() == QLocalSocket::UnconnectedState ) { // could be just the bloody timer
            fError = fSocket.errorString();
            bail();
        }
    }

    void EtherIPC::getAccounts() {
        if ( !queueRequest(RequestIPC(GetAccountRefs, "personal_listAccounts", QJsonArray())) ) {
            return bail();
        }
    }

    bool EtherIPC::refreshAccount(const QString& hash, int index) {
        QJsonArray params;
        params.append(hash);
        params.append("latest");
        if ( !queueRequest(RequestIPC(GetBalance, "eth_getBalance", params, index)) ) {
            bail();
            return false;
        }

        if ( !queueRequest(RequestIPC(GetTransactionCount, "eth_getTransactionCount", params, index)) ) {
            bail();
            return false;
        }

        return true;
    }

    void EtherIPC::handleAccountDetails() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        QJsonArray refs = jv.toArray();
        int i = 0;
        foreach( QJsonValue r, refs ) {
            const QString hash = r.toString("INVALID");
            fAccountList.append(AccountInfo(hash, QString(), -1));
            refreshAccount(hash, i++);
        }

        done();
    }

    void EtherIPC::handleAccountBalance() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        const QString decStr = Helpers::toDecStr(jv);
        const int index = fActiveRequest.getIndex();
        fAccountList[index].setBalance(decStr);
        emit accountChanged(fAccountList.at(index));

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
        fAccountList[index].setTransactionCount(count);

        emit accountChanged(fAccountList.at(index));
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

    void EtherIPC::deleteAccount(const QString& hash, const QString& password, int index) {
        QJsonArray params;
        params.append(hash);
        params.append(password);        
        if ( !queueRequest(RequestIPC(DeleteAccount, "personal_deleteAccount", params, index)) ) {
            return bail();
        }
    }

    void EtherIPC::handleDeleteAccount() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        const bool result = jv.toBool(false);
        emit deleteAccountDone(result, fActiveRequest.getIndex());
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

        emit getBlockNumberDone(result);
        done();
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

    void EtherIPC::sendTransaction(const QString& from, const QString& to, double value, double gas) {
        if ( value <= 0 ) {
            fError = "Invalid transaction value";
            return bail();
        }

        QJsonArray params;
        const QString valHex = Helpers::toHexWeiStr(value);
        const QString gasHex = Helpers::toHexWeiStr(value);

        QJsonObject p;
        p["from"] = from;
        p["to"] = to;
        p["value"] = valHex;
        if ( gas > 0 ) {
            p["gas"] = gasHex;
        }

        params.append(p);

        if ( !queueRequest(RequestIPC(SendTransaction, "eth_sendTransaction", params)) ) {
            return bail();
        }
    }

    void EtherIPC::handleSendTransaction() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        const QString hash = jv.toString();
        emit sendTransactionDone(hash);
        done();
    }

    int EtherIPC::getConnectionState() const {
        if ( fSocket.state() == QLocalSocket::ConnectedState ) {
            return 1; // TODO: add higher states per peer count!
        }

        return 0;
    }

    void EtherIPC::unlockAccount(const QString& hash, const QString& password, int duration, int index) {
        QJsonArray params;
        params.append(hash);
        params.append(password);

        BigInt::Vin vinVal(duration);
        QString strHex = QString(vinVal.toStr0xHex().data());
        params.append(strHex);

        if ( !queueRequest(RequestIPC(UnlockAccount, "personal_unlockAccount", params, index)) ) {
            return bail();
        }
    }

    void EtherIPC::handleUnlockAccount() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        const bool result = jv.toBool(false);
        emit unlockAccountDone(result, fActiveRequest.getIndex());
        done();
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

        const QString decStr = Helpers::toDecStr(jv);

        emit getGasPriceDone(decStr);
        done();
    }

    quint64 EtherIPC::peerCount() const {
        return fPeerCount;
    }

    void EtherIPC::newPendingTransactionFilter() {
        if ( !queueRequest(RequestIPC(NewPendingTransactionFilter, "eth_newPendingTransactionFilter")) ) {
            return bail();
        }
    }

    void EtherIPC::newBlockFilter() {
        if ( !queueRequest(RequestIPC(NewBlockFilter, "eth_newBlockFilter")) ) {
            return bail();
        }
    }

    void EtherIPC::handleFilter(int &filterID) {
        BigInt::Vin bv;
        if ( !readVin(bv) ) {
            return bail();
        }
        const QString strDec = QString(bv.toStrDec().data());
        filterID = strDec.toInt();

        if ( filterID < 0 ) {
            fError = "Filter ID invalid";
            return bail();
        }

        done();
    }

    void EtherIPC::onTimer() {
        getPeerCount();
        getFilterChanges(NewPendingTransactionFilter, fPendingTransactionsFilterID);
        getFilterChanges(NewBlockFilter, fBlockFilterID);
    }

    void EtherIPC::getFilterChanges(RequestTypes subRequest, int filterID) {
        QJsonArray params;
        BigInt::Vin vinVal(filterID);
        QString strHex = QString(vinVal.toStr0xHex().data());
        params.append(strHex);

        if ( !queueRequest(RequestIPC(NonVisual, GetFilterChanges, "eth_getFilterChanges", params, subRequest)) ) {
            return bail();
        }
    }

    void EtherIPC::handleGetFilterChanges() {
        RequestTypes type = (RequestTypes)fActiveRequest.getIndex();
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        QJsonArray ar = jv.toArray();
        foreach( QJsonValue v, ar ) {
            switch ( type ) {
                case NewPendingTransactionFilter: getTransactionByHash(v.toString()); break;
                case NewBlockFilter: getBlockByHash(v.toString()); break;
                default:
                    fError = "Unknown filter subrequest";
                    return bail();
            }
        }

        done();
    }

    void EtherIPC::uninstallFilter(int filterID) {
        QJsonArray params;
        BigInt::Vin vinVal(filterID);
        QString strHex = QString(vinVal.toStr0xHex().data());
        params.append(strHex);

        if ( !queueRequest(RequestIPC(UninstallFilter, "eth_uninstallFilter", params, filterID)) ) {
            return bail();
        }
    }

    void EtherIPC::handleUninstallFilter() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        if ( fActiveRequest.getIndex() == fBlockFilterID ) {
            fBlockFilterID = -1;
        } else if ( fActiveRequest.getIndex() == fPendingTransactionsFilterID ) {
            fPendingTransactionsFilterID = -1;
        }

        done();
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

    void EtherIPC::bail() {
        qDebug() << "BAIL: " << fError << "\n";
        fTimer.stop();
        fActiveRequest = RequestIPC();
        fRequestQueue.clear();
        emit error();
        emit connectionStateChanged();
        done();
    }

    void EtherIPC::done() {
        if ( !fRequestQueue.isEmpty() ) {
            fActiveRequest = fRequestQueue.first();
            fRequestQueue.removeFirst();
            writeRequest();
        } else {
            fActiveRequest = RequestIPC();
            emit busyChanged(getBusy());
        }
    }

    QJsonObject EtherIPC::methodToJSON(const RequestIPC& request) {
        QJsonObject result;

        result.insert("jsonrpc", QJsonValue("2.0"));
        result.insert("method", QJsonValue(request.getMethod()));
        result.insert("id", QJsonValue(request.getCallID()));
        result.insert("params", QJsonValue(request.getParams()));

        return result;
    }

    bool EtherIPC::queueRequest(const RequestIPC& request) {
        if ( fActiveRequest.burden() == None ) {
            fActiveRequest = request;
            if ( fActiveRequest.burden() == Full ) {
                emit busyChanged(getBusy());
            }
            return writeRequest();
        } else {
            fRequestQueue.append(request);
            return true;
        }
    }

    bool EtherIPC::writeRequest() {
        QJsonDocument doc(methodToJSON(fActiveRequest));
        const QString msg(doc.toJson());

        if ( !fSocket.isWritable() ) {
            fError = "Socket not writeable";
            fCode = 0;
            return false;
        }

        const QByteArray sendBuf = msg.toUtf8();
        const int sent = fSocket.write(sendBuf);

        if ( sent <= 0 ) {
            fError = "Error on socket write: " + fSocket.errorString();
            fCode = 0;
            return false;
        }

        //qDebug() << "sent: " << msg << "\n";

        return true;
    }

    bool EtherIPC::readData() {
        fReadBuffer += QString(fSocket.readAll());

        if ( fReadBuffer.at(0) == '{' && fReadBuffer.at(fReadBuffer.length() - 1) == '}' ) {
            return true;
        }

        return false;
    }

    bool EtherIPC::readReply(QJsonValue& result) {
        if ( !readData() ) {
            return true; // not finished yet
        }

        const QString data = fReadBuffer;
        fReadBuffer.clear();

        if ( data.isEmpty() ) {
            fError = "Error on socket read: " + fSocket.errorString();
            fCode = 0;
            return false;
        }

        //qDebug() << "received: " << data << "\n";

        QJsonParseError parseError;
        QJsonDocument resDoc = QJsonDocument::fromJson(data.toUtf8(), &parseError);

        if ( parseError.error != QJsonParseError::NoError ) {
            qDebug() << data << "\n";
            fError = "Response parse error: " + parseError.errorString();
            fCode = 0;
            return false;
        }

        const QJsonObject obj = resDoc.object();
        const int objID = obj["id"].toInt(-1);

        if ( objID != fActiveRequest.getCallID() ) { // TODO
            fError = "Call number mismatch " + QString::number(objID) + " != " + QString::number(fActiveRequest.getCallID());
            fCode = 0;
            return false;
        }

        result = obj["result"];

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

            fError = "Result object undefined in IPC response";
            qDebug() << data << "\n";
            return false;
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

        switch ( fActiveRequest.getType() ) {
        case NewAccount: {
                handleNewAccount();
                break;
            }
        case DeleteAccount: {
                handleDeleteAccount();
                break;
            }
        case GetBlockNumber: {
                handleGetBlockNumber();
                break;
            }
        case GetAccountRefs: {
                handleAccountDetails();
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
        case UnlockAccount: {
                handleUnlockAccount();
                break;
            }
        case GetGasPrice: {
                handleGetGasPrice();
                break;
            }
        case NewPendingTransactionFilter: {
                handleFilter(fPendingTransactionsFilterID);
                break;
            }
        case NewBlockFilter: {
                handleFilter(fBlockFilterID);
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
        default: qDebug() << "Unknown reply: " << fActiveRequest.getType() << "\n"; break;
        }
    }

}
