#ifndef HELPERS_H
#define HELPERS_H

#include <QString>
#include <QJsonObject>
#include <QNetworkReply>
#include "bigint.h"
#include "types.h"

namespace Etherwall {

    class Helpers {
    public:
        static const QString toDecStr(const QJsonValue &jv);
        static const QString toDecStrEther(const QJsonValue &jv);
        static const QString toDecStr(quint64 val);
        static const QString toHexStr(quint64 val);
        static const QString toHexWeiStr(const QString& val);
        static const QString toHexWeiStr(quint64 val);
        static const QString decStrToHexStr(const QString& dec);
        static const QString weiStrToEtherStr(const QString& wei);
        static BigInt::Rossi decStrToRossi(const QString& dec);
        static BigInt::Rossi etherStrToRossi(const QString& dec);
        static const QString formatEtherStr(const QString& ether);
        static const QJsonArray toQJsonArray(const AccountList& list);
        static quint64 toQUInt64(const QJsonValue& jv);
        static int parseAppVersion(const QString& ver);
        static QJsonObject parseHTTPReply(QNetworkReply *reply);
        static const QString vitalizeAddress(const QString& origAddress);
    };

}

#endif // HELPERS_H
