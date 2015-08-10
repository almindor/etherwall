#include "etheripc.h"

namespace Etherwall {

    EtherIPC::EtherIPC() :
        fSocket(), fCallNum(0) {
    }

    void EtherIPC::connect(const QString& path) {
        fSocket.connectToServer(path);
    }

    const QJsonArray EtherIPC::getAccountRefs() {
        QJsonValue ja = callIPC("personal_listAccounts", QJsonArray());
        return ja.toArray();
    }

    qulonglong EtherIPC::getBalance(const QJsonValue& accountRef) {
        QJsonArray params;
        params.append(accountRef.toString());
        params.append("latest");

        QJsonValue jv = callIPC("eth_getBalance", params);
        return jv.toString().toULongLong(NULL, 16);
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
            throw ("Timeout on socket connect " + fSocket.errorString()).toUtf8().constData();
        }

        const QByteArray sendBuf = msg.toUtf8();
        const int sent = fSocket.write(sendBuf);

        if ( sent <= 0 ) {
            throw ("Error on socket send " + fSocket.errorString()).toUtf8().constData();
        }

        if ( !fSocket.waitForBytesWritten(1000) ) {
            throw ("Timeout on socket send " + fSocket.errorString()).toUtf8().constData();
        }

        //qDebug() << "sent: " << msg << "\n";

        if ( !fSocket.waitForReadyRead(1000) ) {
            throw ("Timeout on socket read " + fSocket.errorString()).toUtf8().constData();
        }

        QByteArray recvBuf = fSocket.read(4096);
        if ( recvBuf.isNull() || recvBuf.isEmpty() ) {
            throw ("Error on socket read " + fSocket.errorString()).toUtf8().constData();
        }

        //qDebug() << "received: " << recvBuf << "\n";

        QJsonParseError parseError;
        QJsonDocument resDoc = QJsonDocument::fromJson(recvBuf, &parseError);

        if ( parseError.error != QJsonParseError::NoError ) {
            throw ("Error on response JSON parse: " + parseError.errorString()).toUtf8().constData();
        }

        const QJsonObject obj = resDoc.object();
        const QJsonValue objID = obj["id"];

        if ( objID.toInt(-1) != fCallNum++ ) {
            throw "Call response ID mismatch";
        }

        QJsonValue result = obj["result"];

        if ( result.isUndefined() || result.isNull() ) {
            throw "Call response object null or undefined";
        }

        return result;
    }

}
