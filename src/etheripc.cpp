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
#include "bigint.h"
#include <QJsonDocument>
#include <QJsonValue>

namespace Etherwall {

// *************************** RequestIPC **************************** //

    RequestIPC::RequestIPC(RequestTypes type, const QString method, const QJsonArray params, int index) :
        fType(type), fMethod(method), fParams(params), fIndex(index)
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

// *************************** EtherIPC **************************** //

    EtherIPC::EtherIPC() :
        fSocket(), fCallNum(0), fLocale(), fError(), fCode(0), fBusy(false), fAccountList(), fRequestQueue() {

        connect(&fSocket, (void (QLocalSocket::*)(QLocalSocket::LocalSocketError))&QLocalSocket::error, this, &EtherIPC::onSocketError);
        connect(&fSocket, &QLocalSocket::readyRead, this, &EtherIPC::onSocketReadyRead);
        connect(&fSocket, &QLocalSocket::connected, this, &EtherIPC::connectToServerDone);
    }

    void EtherIPC::start(const QString& ipcPath) {
        connectToServer(ipcPath);
    }

    bool EtherIPC::getBusy() const {
        return fBusy;
    }

    const QString& EtherIPC::getError() const {
        return fError;
    }

    int EtherIPC::getCode() const {
        return fCode;
    }

    void EtherIPC::closeApp() {
        fSocket.abort();
        thread()->quit();
    }

    void EtherIPC::connectToServer(const QString& path) {
        fSocket.connectToServer(path);
    }

    void EtherIPC::getAccounts() {
        if ( !writeRequest(RequestIPC(GetAccountRefs, "personal_listAccounts", QJsonArray())) ) {
            return bail();
        }
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
            QJsonArray params;
            params.append(hash);
            params.append("latest");
            if ( !writeRequest(RequestIPC(GetBalance, "eth_getBalance", params, i)) ) {
                return bail();
            }

            if ( !writeRequest(RequestIPC(GetTransactionCount, "eth_getTransactionCount", params, i++)) ) {
                return bail();
            }
        }

        done();
    }

    void EtherIPC::handleAccountBalance() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        std::string hexStr = jv.toString("0x0").remove(0, 2).toStdString();
        const BigInt::Vin bv(hexStr, 16);
        QString decStr = QString(bv.toStrDec().data());

        const int dsl = decStr.length();
        if ( dsl <= 18 ) {
            decStr.prepend(QString(18 - dsl, '0'));
        }
        decStr.insert(dsl - 18, fLocale.decimalPoint());
        fAccountList[index()].setBalance(decStr);

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
        fAccountList[index()].setTransactionCount(count);

        if ( index() + 1 == fAccountList.length() ) {
            emit getAccountsDone(fAccountList);
        }
        done();
    }

    void EtherIPC::newAccount(const QString& password, int index) {
        QJsonArray params;
        params.append(password);
        if ( !writeRequest(RequestIPC(NewAccount, "personal_newAccount", params, index)) ) {
            return bail();
        }
    }

    void EtherIPC::handleNewAccount() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        const QString result = jv.toString();
        emit newAccountDone(result, index());
        done();
    }

    void EtherIPC::deleteAccount(const QString& hash, const QString& password, int index) {
        QJsonArray params;
        params.append(hash);
        params.append(password);        
        if ( !writeRequest(RequestIPC(DeleteAccount, "personal_deleteAccount", params, index)) ) {
            return bail();
        }
    }

    void EtherIPC::handleDeleteAccount() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        const bool result = jv.toBool(false);
        emit deleteAccountDone(result, index());
        done();
    }

    void EtherIPC::getBlockNumber() {
        if ( !writeRequest(RequestIPC(GetBlockNumber, "eth_blockNumber")) ) {
            return bail();
        }
    }

    void EtherIPC::handleGetBlockNumber() {
        QJsonValue jv;
        if ( !readReply(jv) ) {
            return bail();
        }

        std::string hexStr = jv.toString("0x0").remove(0, 2).toStdString();
        const BigInt::Vin bv(hexStr, 16);

        emit getBlockNumberDone(bv.toUlong());
        done();
    }

    int EtherIPC::index() const {
        return fRequestQueue.length() > 0 ? fRequestQueue.first().getIndex() : -1;
    }

    RequestTypes EtherIPC::requestType() const {
        return fRequestQueue.length() > 0 ? fRequestQueue.first().getType() : NoRequest;
    }

    void EtherIPC::bail() {
        fRequestQueue.clear();
        emit error(fError, fCode);
        done();
    }

    void EtherIPC::done() {
        fBusy = false;

        fRequestQueue.removeFirst();

        if ( !fRequestQueue.isEmpty() ) {
            //qDebug() << "Queue not empty, calling: " << fRequestQueue.first().getMethod() << "\n";

            if ( !writeRequest(fRequestQueue.first(), true) ) { // makes it busy again!
                 return bail();
            }
        }

        emit busyChanged(fBusy);
    }

    QJsonObject EtherIPC::methodToJSON(const RequestIPC& request) {
        QJsonObject result;

        result.insert("jsonrpc", QJsonValue("2.0"));
        result.insert("method", QJsonValue(request.getMethod()));
        result.insert("id", QJsonValue(fCallNum));
        result.insert("params", QJsonValue(request.getParams()));

        return result;
    }

    bool EtherIPC::writeRequest(const RequestIPC& request, bool fromQueue) {
        if ( !fromQueue ) {
            fRequestQueue.append(request);
            if ( fBusy ) { // queued, it'll be handled
                return true;
            }

            fBusy = true;
            emit busyChanged(fBusy);
        }

        QJsonDocument doc(methodToJSON(request));
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

    bool EtherIPC::readReply(QJsonValue& result) {
        QByteArray recvBuf = fSocket.read(4096);
        if ( recvBuf.isNull() || recvBuf.isEmpty() ) {
            fError = "Error on socket read: " + fSocket.errorString();
            fCode = 0;
            return false;
        }

        //qDebug() << "received: " << recvBuf << "\n";

        QJsonParseError parseError;
        QJsonDocument resDoc = QJsonDocument::fromJson(recvBuf, &parseError);

        if ( parseError.error != QJsonParseError::NoError ) {
            fError = "Response parse error: " + parseError.errorString();
            fCode = 0;
            return false;
        }

        const QJsonObject obj = resDoc.object();
        const QJsonValue objID = obj["id"];

        if ( objID.toInt(-1) != fCallNum++ ) {
            fError = "Call number mismatch";
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

                if ( fCode == -32603 ) { // wrong password
                    fError = "Wrong password [" + fError + "]";
                }

                return false;
            }

            fError = "Result object undefined in IPC response";
            return false;
        }

        return true;
    }

    void EtherIPC::onSocketError(QLocalSocket::LocalSocketError err) {
        fError = fSocket.errorString();
        fCode = err;
        emit error(fSocket.errorString(), err);
        fBusy = false;
        emit busyChanged(fBusy);
    }

    void EtherIPC::onSocketReadyRead() {
        switch ( requestType() ) {
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
        default: break;
        }
    }

}
