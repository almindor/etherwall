#include "helpers.h"
#include "etherlog.h"
#include <QJsonParseError>
#include <QCryptographicHash>
#include <QBitArray>
#include <QDataStream>
#include <QSettings>
#include <QFile>
#include <QDataStream>

namespace Etherwall {

// ***************************** Helpers ***************************** //

    const QString Helpers::hexPrefix(const QString& val) {
        return val.indexOf("0x") == 0 ? val : ("0x" + val);
    }

    const QString Helpers::clearHexPrefix(const QString &val)
    {
        if ( val.startsWith("0x") || val.startsWith("0X") ) {
            return val.mid(2);
        }

        return val;
    }

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

        return res;
    }

    const QString Helpers::toHexWeiStr(quint64 val) {
        BigInt::Vin vinVal(val);
        return QString(vinVal.toStr0xHex().data());
    }

    const QString Helpers::decStrToHexStr(const QString &dec) {
        BigInt::Vin vinVal(dec.toStdString(), 10);
        return QString(vinVal.toStr0xHex().data());
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

    const QString Helpers::vitalizeAddress(const QString& origAddress) {
        // TODO: revisit
        return origAddress.toLower(); // https://github.com/ethereum/EIPs/issues/55 is a mess right now, the hash is not specified and implementations vary

        /*
        QString address = origAddress.toLower();
        if ( address.indexOf("0x") == 0 ) {
            address = address.remove(0, 2);
        }

        if ( address.length() != 40 ) {
            return origAddress;
        }

        const QByteArray byteAddress = address.toUtf8();
        const QByteArray hashed = QCryptographicHash::hash(byteAddress, QCryptographicHash::Sha3_256);
        const QString hashStr = QString(hashed.toHex());

        QString result = "";
        int i = 0;
        foreach ( const QChar c, address ) {
            if ( c >= '0' && c <= '9' ) {
                result += c;
            } else {
                bool ok = false;
                int cVal = hashStr.mid(i, 1).toInt(&ok, 16);
                if ( !ok ) {
                    qDebug() << "error parsing int\n";
                    return "0x0";
                }

                result += cVal > 7 ? c.toUpper() : c.toLower();
            }

            i++;
        }

        return "0x" + result;
        */
    }

    const QString Helpers::networkPostfix(int network)
    {
        switch ( network ) {
            case 1: return "/eth/homestead";
            case 3: return "/eth/ropsten";
            case 4: return "/eth/rinkeby";
        }

        return "/unknown/unknown";
    }

    const QByteArray Helpers::exportSettings() {
        const QSettings settings;
        QByteArray result;

        foreach ( const QString key, settings.allKeys() ) {
            if ( key.startsWith("alias/") || key.startsWith("geth/") || key.startsWith("ipc/") || key.startsWith("program") ||
                 key.startsWith("contracts/") || key.startsWith("filters/") || key.startsWith("transactions") ) {
                result += key.toUtf8() + '\0' + settings.value(key, "invalid").toString().toUtf8() + '\0';
            }
        }

        return result;
    }

    void Helpers::importSettings(const QByteArray &data) {
        QSettings settings;
        QByteArray key;
        QByteArray value;
        QByteArray* word = &key;

        // data consists of key\0value\0 combinations
        foreach ( const char c, data ) {
            if ( c != 0 ) {
                word->append(c);
            } else { // delimiter reached
                if ( word == &value ) {
                    settings.setValue(QString(key), QString(value));
                    word = &key;
                    value.clear();
                    key.clear();
                } else {
                    word = &value;
                }
            }
        }

        settings.sync();
    }

    const QString Helpers::getAddressFilename(const QDir& keystore, QString& address) {
        if ( !keystore.exists() ) {
            throw QString("Address keystore directory does not exist: " + keystore.absolutePath());
        }

        address = address.toLower();
        if ( address.startsWith("0x") ) {
            address = address.remove(0, 2);
        }

        const QStringList nameFilter("UTC*");

        foreach ( const QString fileName, keystore.entryList(nameFilter) ) {
            QFile file(keystore.filePath(fileName));
            file.open(QFile::ReadOnly);
            const QByteArray raw = file.readAll();
            file.close();
            const QJsonDocument doc = QJsonDocument::fromJson(raw);
            const QJsonObject contents = doc.object();

            if ( contents.value("address").toString("invalid").toLower() == address ) {
                return fileName;
            }
        }

        return QString();
    }

    const QString Helpers::exportAddress(const QDir& keystore, QString& address) {
        if ( !keystore.exists() ) {
            throw QString("Address keystore directory does not exist: " + keystore.absolutePath());
        }

        address = address.toLower();
        if ( address.startsWith("0x") ) {
            address = address.remove(0, 2);
        }

        const QStringList nameFilter("UTC*");

        foreach ( const QString fileName, keystore.entryList(nameFilter) ) {
            QFile file(keystore.filePath(fileName));
            file.open(QFile::ReadOnly);
            const QByteArray raw = file.readAll();
            file.close();
            const QJsonDocument doc = QJsonDocument::fromJson(raw);
            const QJsonObject contents = doc.object();

            if ( contents.value("address").toString("invalid").toLower() == address ) {
                return QString(raw);
            }
        }

        return QString();
    }

    const QByteArray Helpers::exportAddresses(const QDir& keystore) {
        if ( !keystore.exists() ) {
            throw QString("Address keystore directory does not exist: " + keystore.absolutePath());
        }

        QByteArray result;
        QDataStream stream(&result, QIODevice::WriteOnly);
        const QStringList nameFilter("UTC*");

        foreach ( const QString fileName, keystore.entryList(nameFilter) ) {
            QFile file(keystore.filePath(fileName));
            file.open(QFile::ReadOnly);
            const QByteArray raw = file.readAll();
            file.close();
            const QJsonDocument doc = QJsonDocument::fromJson(raw);
            const QJsonObject contents = doc.object();
            QJsonObject wrapper;
            wrapper["address"] = contents.value("address").toString("invalid");
            wrapper["filename"] = fileName;
            wrapper["contents"] = contents;
            const QByteArray binary = QJsonDocument(wrapper).toBinaryData();

            quint32 qSize = binary.size();
            stream << qSize;
            stream << binary;
        }

        return result;
    }

    void Helpers::importAddresses(QByteArray &data, const QDir& keystore) {
        QStringList existingList;
        const QStringList nameFilter("UTC*");
        foreach ( const QString fileName, keystore.entryList(nameFilter) ) {
            QFile file(keystore.filePath(fileName));
            file.open(QFile::ReadOnly);
            const QByteArray raw = file.readAll();
            file.close();
            const QJsonDocument doc = QJsonDocument::fromJson(raw);
            const QJsonObject contents = doc.object();
            existingList.append(contents.value("address").toString("invalid"));
        }

        QDataStream stream(&data, QIODevice::ReadOnly);
        while ( !stream.atEnd() ) {
            quint32 size = 0;
            stream >> size;

            if ( size == 0 ) {
                throw QString("Invalid size in address datastream: " + QString::number(size, 10));
            }

            QByteArray raw(size, '\0');
            stream >> raw;
            const QJsonObject wrapper = QJsonDocument::fromBinaryData(raw).object();

            // check for existing address NOTE: filename can differ! using address to address check
            const QString address = wrapper.value("address").toString();
            if ( existingList.contains(address, Qt::CaseInsensitive) ) {
                continue;
            }

            const QJsonObject contents = wrapper.value("contents").toObject();
            QFile file(keystore.filePath(wrapper.value("filename").toString("invalid")));
            file.open(QFile::WriteOnly);
            file.write(QJsonDocument(contents).toJson(QJsonDocument::Compact));
            file.close();
        }
    }

    const QByteArray Helpers::createBackup(const QDir& keystore) {
        const QByteArray settingsData = exportSettings();
        const QByteArray addressData = exportAddresses(keystore);
        QByteArray testnetData;

        QDir testnet(keystore);
        // can't use && because C++ doesn't enforce execution order AFAIK
        if ( testnet.cd("../rinkeby") ) {
            if ( testnet.cd("keystore") ) {
                testnetData = exportAddresses(testnet);
            }
        }

        QByteArray all; // all without checksum
        QByteArray result; // full result with 16bit checksum
        QDataStream allStream(&all, QIODevice::WriteOnly);
        QDataStream resultStream(&result, QIODevice::WriteOnly);
        quint32 segmentSize;

        segmentSize = settingsData.size();
        allStream << segmentSize;
        allStream << settingsData;
        segmentSize = addressData.size();
        allStream << segmentSize;
        allStream << addressData;
        // add testnet addresses if present
        if ( testnetData.size() > 0 ) {
            segmentSize = testnetData.size();
            allStream << segmentSize;
            allStream << testnetData;
        }

        quint32 allSize = all.size();
        quint16 crc = qChecksum(all.data(), all.size());
        resultStream << allSize;
        resultStream << crc;
        resultStream << all;

        return qCompress(result, 9);
    }

    void Helpers::restoreBackup(QByteArray& data, const QDir& keystore) {
        QByteArray raw = qUncompress(data);
        QDataStream totalStream(&raw, QIODevice::ReadOnly);

        quint32 allSize;
        quint16 crc;
        totalStream >> allSize;
        totalStream >> crc;
        QByteArray all(allSize, '\0');
        totalStream >> all;

        quint32 segmentSize;
        quint16 checkSum = qChecksum(all.data(), all.size());

        // do CRC check
        if ( crc != checkSum ) {
            throw QString("CRC check mismatch: " + QString::number(crc) + " != " + QString::number(checkSum));
        } else if ( !totalStream.atEnd() ) {
            throw QString("Unexpected data at end of restore stream");
        }

        QByteArray testnetData;

        QDataStream allStream(&all, QIODevice::ReadOnly);
        allStream >> segmentSize;
        QByteArray settingsData(segmentSize, '\0');
        allStream >> settingsData;
        allStream >> segmentSize;
        QByteArray addressData(segmentSize, '\0');
        allStream >> addressData;
        if ( !allStream.atEnd() ) { // testnet addresses are present
            allStream >> segmentSize;
            testnetData = QByteArray(segmentSize, '\0');
            allStream >> testnetData;
        }

        importSettings(settingsData);
        importAddresses(addressData, keystore);
        QDir testnet(keystore);
        // can't use && because C++ doesn't enforce execution order AFAIK
        if ( testnetData.size() > 0 ) {
            if ( !testnet.cd("..") ) throw QString("Unable to reach main datadir");
            testnet.mkdir("rinkeby");
            if ( !testnet.cd("rinkeby") ) throw QString("Unable to reach rinkeby datadir");;
            testnet.mkdir("keystore");
            if ( !testnet.cd("keystore") ) throw QString("Unable to reach testnet keystore");;
            importAddresses(testnetData, testnet);
        }
    }

    // ***************************** QmlHelpers ***************************** //

    QmlHelpers::QmlHelpers() : QObject(0) {

    }

    bool QmlHelpers::checkAddress(const QString& origAddress) const {
        return true;

        // TODO: revisit (see vitalizeAddress)

        // return origAddress == Helpers::vitalizeAddress(origAddress);
    }

    const QString QmlHelpers::localURLToString(const QUrl& url) const {
        return url.toLocalFile();
    }

    const QString QmlHelpers::exportAddress(const QString& address, bool testnet) const {
        const QSettings settings;
        QDir keystore(settings.value("geth/datadir").toString());
        if ( testnet ) {
            keystore.cd("rinkeby");
        }
        keystore.cd("keystore");

        QString tmp(address);
        return Helpers::exportAddress(keystore, tmp);
    }

    int QmlHelpers::parseAppVersion(const QString& ver) const {
        return Helpers::parseAppVersion(ver);
    }

}
