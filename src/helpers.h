#ifndef HELPERS_H
#define HELPERS_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QNetworkReply>
#include <QDir>
#include "bigint.h"
#include "types.h"

namespace Etherwall {

    class Helpers
    {
    public:
        static const QString hexPrefix(const QString& val);
        static const QString clearHexPrefix(const QString& val);
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
        static const QString networkPostfix(int network);
        static const QByteArray exportSettings();
        static void importSettings(const QByteArray& data);
        static const QString getAddressFilename(const QDir& keystore, QString& address);
        static const QString exportAddress(const QDir& keystore, QString& address);
        static const QByteArray exportAddresses(const QDir& keystore);
        static void importAddresses(QByteArray& data, const QDir& keystore);
        static const QByteArray createBackup(const QDir& keystore);
        static void restoreBackup(QByteArray& data, const QDir& keystore);
    };

    class QmlHelpers : public QObject
    {
        Q_OBJECT
    public:
        QmlHelpers();
        Q_INVOKABLE bool checkAddress(const QString& origAddress) const;
        Q_INVOKABLE const QString localURLToString(const QUrl& url) const;
        Q_INVOKABLE const QString exportAddress(const QString& address, bool testnet) const;
        Q_INVOKABLE int parseAppVersion(const QString& ver) const;
    };

}

#endif // HELPERS_H
