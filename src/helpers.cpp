#include "helpers.h"
#include "etherlog.h"
#include <QJsonParseError>

namespace Etherwall {


// ***************************** Helpers ***************************** //

    const QString Helpers::toDecStr(const QJsonValue& jv) {
        std::string hexStr = jv.toString("0x0").remove(0, 2).toStdString();
        const BigInt::Vin bv(hexStr, 16);
        QString decStr = QString(bv.toStrDec().data());

        return decStr;
    }

    const QString Helpers::toDecStrEther(const QJsonValue& jv) {
        QString decStr = toDecStr(jv);

        int dsl = decStr.length();
        if ( dsl <= 18 ) {
            decStr.prepend(QString(19 - dsl, '0'));
            dsl = decStr.length();
        }
        decStr.insert(dsl - 18, '.');
        return decStr;
    }

    const QString Helpers::toDecStr(quint64 val) {
        BigInt::Vin vinVal(val);
        return QString(vinVal.toStrDec().data());
    }

    const QString Helpers::toHexStr(quint64 val) {
        BigInt::Vin vinVal(val);
        return QString(vinVal.toStr0xHex().data());
    }

    const QString Helpers::toHexWeiStr(const QString& val) {
        QString decStr = val;

        int diff = 18;
        int n = decStr.indexOf('.');
        if ( n >= 0 ) {
            decStr.replace(".", "");
            diff = 18 - (decStr.length() - n);
        }

        for ( int i = 0; i < diff; i++ ) {
            decStr.append('0');
        }

        BigInt::Vin vinVal(decStr.toUtf8().data(), 10);
        QString res = QString(vinVal.toStr0xHex().data());

        return QString(vinVal.toStr0xHex().data());
    }

    const QString Helpers::toHexWeiStr(quint64 val) {
        BigInt::Vin vinVal(val);
        return QString(vinVal.toStr0xHex().data());
    }

    const QString Helpers::decStrToHexStr(const QString &dec) {
        BigInt::Vin vinVal(dec.toStdString(), 10);
        return QString(vinVal.toStrDec().data());
    }

    const QString Helpers::weiStrToEtherStr(const QString& wei) {
        QString weiStr = wei;
        while ( weiStr.length() < 18 ) {
            weiStr.insert(0, '0');
        }

        weiStr.insert(weiStr.length() - 18, '.');
        if ( weiStr.at(0) == '.' ) {
            weiStr.insert(0, '0');
        }
        return weiStr;
    }

    BigInt::Rossi Helpers::decStrToRossi(const QString& dec) {
        return BigInt::Rossi(dec.toStdString(), 10);
    }

    BigInt::Rossi Helpers::etherStrToRossi(const QString& dec) {
        QString decStr = dec;

        int diff = 18;
        int n = decStr.indexOf('.');
        if ( n >= 0 ) {
            decStr.replace(".", "");
            diff = 18 - (decStr.length() - n);
        }

        for ( int i = 0; i < diff; i++ ) {
            decStr.append('0');
        }

        return decStrToRossi(decStr);
    }

    const QString Helpers::formatEtherStr(const QString& ether) {
        QString decStr = ether;

        int n = decStr.indexOf('.');
        int diff;
        if ( n < 0 ) {
            decStr.append('.');
            n = decStr.indexOf('.');
            diff = 18;
        } else {
            diff = 18 - (decStr.length() - n - 1);
        }

        for ( int i = 0; i < diff; i++ ) {
            decStr.append('0');
        }

        return decStr;
    }

    const QJsonArray Helpers::toQJsonArray(const AccountList& list) {
        QJsonArray result;

        foreach ( const AccountInfo info, list ) {
            const QString hash = info.value(HashRole).toString();
            result.append(hash);
        }

        return result;
    }

    quint64 Helpers::toQUInt64(const QJsonValue& jv) {
        std::string hexStr = jv.toString("0x0").remove(0, 2).toStdString();
        BigInt::Vin vin(hexStr, 16);
        return vin.toUlong();
    }

    int Helpers::parseAppVersion(const QString &ver) {
        QRegExp reg("([0-9]+)\\.([0-9]+)\\.([0-9]+).*$");
        reg.indexIn(ver);
        if ( reg.captureCount() == 3 ) try {
            return reg.cap(1).toInt() * 100000 + reg.cap(2).toInt() * 1000 + reg.cap(3).toInt();
        } catch ( ... ) {
            return 0;
        }

        return 0;
    }

    QJsonObject Helpers::parseHTTPReply(QNetworkReply *reply) {
        if ( reply == NULL ) {
            EtherLog::logMsg("Undefined reply", LS_Error);
            return QJsonObject();
        }

        const QByteArray data = reply->readAll();
        EtherLog::logMsg("HTTP Post reply: " + data, LS_Debug);

        QJsonParseError parseError;
        const QJsonDocument resDoc = QJsonDocument::fromJson(data, &parseError);

        if ( parseError.error != QJsonParseError::NoError ) {
            EtherLog::logMsg("HTTP Response parse error: " + parseError.errorString(), LS_Error);
            return QJsonObject();
        }

        return resDoc.object();
    }

}
