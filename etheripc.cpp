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

    void EtherIPC::connect(const QString& path) {
        fSocket.connectToServer(path);
    }

    const QJsonArray EtherIPC::getAccountRefs() {
        QJsonValue ja = callIPC("personal_listAccounts", QJsonArray());
        return ja.toArray();
    }

    const QString EtherIPC::getBalance(const QJsonValue& accountRef, const QString& block) {
        QJsonArray params;
        params.append(accountRef.toString());
        params.append(block);

        QJsonValue jv = callIPC("eth_getBalance", params);

        std::string hexStr = jv.toString("0x0").remove(0, 2).toStdString();
        const BigInt::Vin result(hexStr, 16);
        QString decStr = QString(result.toStrDec().data());

        const int dsl = decStr.length();
        if ( dsl <= 18 ) {
            decStr.prepend(QString(18 - dsl, '0'));
        }
        decStr.insert(dsl - 18, fLocale.decimalPoint());

        return decStr;
    }

    quint64 EtherIPC::getTransactionCount(const QJsonValue& accountRef, const QString& block) {
        QJsonArray params;
        params.append(accountRef.toString());
        params.append(block);

        QJsonValue jv = callIPC("eth_getTransactionCount", params);

        std::string hexStr = jv.toString("0x0").remove(0, 2).toStdString();
        const BigInt::Vin result(hexStr, 16);
        return result.toUlong();
    }

    const QString EtherIPC::newAccount(const QString& password) {
        QJsonArray params;
        params.append(password);

        QJsonValue jv = callIPC("personal_newAccount", params);
        const QString result = jv.toString();

        return result;
    }

    bool EtherIPC::deleteAccount(const QString& hash, const QString& password) {
        QJsonArray params;
        params.append(hash);
        params.append(password);

        QJsonValue jv = callIPC("personal_deleteAccount", params);
        const bool result = jv.toBool(false);

        return result;
    }

    QJsonObject EtherIPC::methodToJSON(const QString& method, const QJsonArray& params) {
        QJsonObject result;

        result.insert("jsonrpc", QJsonValue("2.0"));
        result.insert("method", QJsonValue(method));
        result.insert("id", QJsonValue(fCallNum));
        result.insert("params", QJsonValue(params));

        return result;
    }

    const QJsonValue EtherIPC::callIPC(const QString& method, const QJsonArray& params) {
        QJsonDocument doc(methodToJSON(method, params));
        const QString msg(doc.toJson());

        if ( !fSocket.waitForConnected(1000) ) {
            fError = "Timeout on socket connect: " + fSocket.errorString();
            throw std::exception();
        }

        const QByteArray sendBuf = msg.toUtf8();
        const int sent = fSocket.write(sendBuf);

        if ( sent <= 0 ) {
            fError = "Error on socket write: " + fSocket.errorString();
            throw std::exception();
        }

        if ( !fSocket.waitForBytesWritten(1000) ) {
            fError = "Timeout on socket write";
            throw std::exception();
        }

        //qDebug() << "sent: " << msg << "\n";

        if ( !fSocket.waitForReadyRead(10000) ) {
            fError = "Timeout on socket read";
            throw std::exception();
        }

        QByteArray recvBuf = fSocket.read(4096);
        if ( recvBuf.isNull() || recvBuf.isEmpty() ) {
            fError = "Error on socket read: " + fSocket.errorString();
            throw std::exception();
        }

        //qDebug() << "received: " << recvBuf << "\n";

        QJsonParseError parseError;
        QJsonDocument resDoc = QJsonDocument::fromJson(recvBuf, &parseError);

        if ( parseError.error != QJsonParseError::NoError ) {
            fError = "Response parse error: " + parseError.errorString();
            throw std::exception();
        }

        const QJsonObject obj = resDoc.object();
        const QJsonValue objID = obj["id"];

        if ( objID.toInt(-1) != fCallNum++ ) {
            fError = "Call number mismatch";
            throw std::exception();
        }

        QJsonValue result = obj["result"];

        if ( result.isUndefined() || result.isNull() ) {
            fError = "Result object undefined in IPC response";
            throw std::exception();
        }

        return result;
    }

}
