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

    EtherIPC::EtherIPC() :
        fSocket(), fCallNum(0), fLocale(), fError() {
    }

    const QString& EtherIPC::getError() const {
        return fError;
    }

    void EtherIPC::setWorker(QThread* worker) {
        moveToThread(worker);
        fSocket.moveToThread(worker);
    }

    void EtherIPC::closeApp() {
        fSocket.abort();
        thread()->quit();
    }

    void EtherIPC::connectToServer(const QString& path) {
        fSocket.connectToServer(path);

        if ( !fSocket.waitForConnected(2000) ) {
            fError = "Socket connection timeout";
            emit error(fError);
        }

        emit connectToServerDone();
    }

    void EtherIPC::getAccounts() {
        QJsonArray refs;
        if ( !getAccountRefs(refs) ) {
            emit error(fError);
            return;
        }

        AccountList list;
        foreach( QJsonValue r, refs ) {
            const QString hash = r.toString("INVALID");
            QString balance;
            if ( !getBalance(r, balance) ) {
                emit error(fError);
                return;
            }

            quint64 transCount;
            if ( !getTransactionCount(r, transCount) ) {
                emit error(fError);
                return;
            }

            list.append(AccountInfo(hash, balance, transCount));
        }

        emit getAccountsDone(list);
    }

    void EtherIPC::newAccount(const QString& password, int index) {
        QJsonArray params;
        params.append(password);

        QJsonValue jv;
        if ( !callIPC("personal_newAccount", params, jv) ) {
            emit error(fError);
            return;
        }

        const QString result = jv.toString();
        emit newAccountDone(result, index);
    }

    void EtherIPC::deleteAccount(const QString& hash, const QString& password, int index) {
        QJsonArray params;
        params.append(hash);
        params.append(password);        

        QJsonValue jv;
        if ( !callIPC("personal_deleteAccount", params, jv) ) {
            emit error(fError);
            return;
        }

        const bool result = jv.toBool(false);
        emit deleteAccountDone(result, index);
    }

    bool EtherIPC::getAccountRefs(QJsonArray& result) {
        QJsonValue ja;
        if ( !callIPC("personal_listAccounts", QJsonArray(), ja ) ) {
            return false;
        }

        result = ja.toArray();
        return true;
    }

    bool EtherIPC::getBalance(const QJsonValue& accountRef, QString& result, const QString& block) {
        QJsonArray params;
        params.append(accountRef.toString());
        params.append(block);

        QJsonValue jv;
        if ( !callIPC("eth_getBalance", params, jv) ) {
             return false;
        }

        std::string hexStr = jv.toString("0x0").remove(0, 2).toStdString();
        const BigInt::Vin bv(hexStr, 16);
        QString decStr = QString(bv.toStrDec().data());

        const int dsl = decStr.length();
        if ( dsl <= 18 ) {
            decStr.prepend(QString(18 - dsl, '0'));
        }
        decStr.insert(dsl - 18, fLocale.decimalPoint());

        result = decStr;
        return true;
    }

    bool EtherIPC::getTransactionCount(const QJsonValue& accountRef, quint64& result, const QString& block) {
        QJsonArray params;
        params.append(accountRef.toString());
        params.append(block);

        QJsonValue jv;
        if ( !callIPC("eth_getTransactionCount", params, jv) ) {
            return false;
        }

        std::string hexStr = jv.toString("0x0").remove(0, 2).toStdString();
        const BigInt::Vin bv(hexStr, 16);

        result = bv.toUlong();
        return true;
    }


    QJsonObject EtherIPC::methodToJSON(const QString& method, const QJsonArray& params) {
        QJsonObject result;

        result.insert("jsonrpc", QJsonValue("2.0"));
        result.insert("method", QJsonValue(method));
        result.insert("id", QJsonValue(fCallNum));
        result.insert("params", QJsonValue(params));

        return result;
    }

    bool EtherIPC::callIPC(const QString& method, const QJsonArray& params, QJsonValue& result) {
        QJsonDocument doc(methodToJSON(method, params));
        const QString msg(doc.toJson());

        if ( !fSocket.waitForConnected(1000) ) {
            fError = "Timeout on socket connect: " + fSocket.errorString();
            return false;
        }

        const QByteArray sendBuf = msg.toUtf8();
        const int sent = fSocket.write(sendBuf);

        if ( sent <= 0 ) {
            fError = "Error on socket write: " + fSocket.errorString();
            return false;
        }

        if ( !fSocket.waitForBytesWritten(1000) ) {
            fError = "Timeout on socket write";
            return false;
        }

        //qDebug() << "sent: " << msg << "\n";

        if ( !fSocket.waitForReadyRead(10000) ) {
            fError = "Timeout on socket read";
            return false;
        }

        QByteArray recvBuf = fSocket.read(4096);
        if ( recvBuf.isNull() || recvBuf.isEmpty() ) {
            fError = "Error on socket read: " + fSocket.errorString();
            return false;
        }

        //qDebug() << "received: " << recvBuf << "\n";

        QJsonParseError parseError;
        QJsonDocument resDoc = QJsonDocument::fromJson(recvBuf, &parseError);

        if ( parseError.error != QJsonParseError::NoError ) {
            fError = "Response parse error: " + parseError.errorString();
            return false;
        }

        const QJsonObject obj = resDoc.object();
        const QJsonValue objID = obj["id"];

        if ( objID.toInt(-1) != fCallNum++ ) {
            fError = "Call number mismatch";
            return false;
        }

        result = obj["result"];

        if ( result.isUndefined() || result.isNull() ) {
            if ( obj.contains("error") && obj["error"].toObject().contains("message") ) {
                fError = obj["error"].toObject()["message"].toString();
                return false;
            }

            fError = "Result object undefined in IPC response";
            return false;
        }

        return true;
    }

}
