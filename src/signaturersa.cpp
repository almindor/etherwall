#include "signaturersa.h"
#include <QDebug>
#include <QRegExp>
#include <QFile>

namespace PGP {

    const int MAX_HEADER_BYTES = 1024;
    const quint32 CRC24_INIT = 0xB704CEL;
    const quint32 CRC24_POLY = 0x1864CFBL;

    SignatureRSA::SignatureRSA(const QString& filePath)
    {
        QFile file(filePath);

        if ( !file.open(QIODevice::ReadOnly) ) {
            throw QString("Unable to open armored signature file");
        }

        const QByteArray armored = file.readAll();

        read(armored);
    }

    SignatureRSA::SignatureRSA(const QByteArray& armored)
    {
        read(armored);
    }

    const QByteArray SignatureRSA::hex() const
    {
        return fSignatureData.toHex();
    }

    const char* SignatureRSA::raw() const
    {
        return fSignatureData.constData();
    }

    int SignatureRSA::rawSize() const
    {
        return fSignatureData.size();
    }

    const QByteArray SignatureRSA::ending() const
    {
        QByteArray ending;
        ending.append(fVersion); // version 4 only
        ending.append(fType); // type 0 only
        ending.append(fPubKeyType); // pub key type only rsa (1)
        ending.append(fHashType); // hash type only sha256 (8)
        QString hashedSizeHex = QString("%1").arg(fHashedSubPackets.size(), 4, 16, QChar('0'));
        ending += QByteArray::fromHex(hashedSizeHex.toUtf8()); // hashed size
        ending += fHashedSubPackets; // hashed sub packets such as timestamp and expiry
        QString totalSizeHex = QString("%1").arg(ending.size(), 8, 16, QChar('0')); // size of "trailer" without 04ff
        ending.append(fVersion); // version again?
        ending.append((char) 0xff); // no idea
        ending += QByteArray::fromHex(totalSizeHex.toUtf8()); // total size

        return ending;
    }

    void SignatureRSA::read(const QByteArray& armored)
    {
        const QByteArray rawData = getRawData(armored);

        if ( rawData.size() < 1 ) {
            throw toError("Invalid input");
        }

        fSignatureData = extractSignature(rawData);
    }

    const QByteArray SignatureRSA::getRawData(const QByteArray& armored) const
    {
        const QRegExp regexp("^\\n?-----BEGIN PGP SIGNATURE-----\\n?[\\w-:.@\\s]*\\n\\n([0-9a-zA-Z+=\\/\n]+)\n([0-9a-zA-Z+=\\/\n]+)\\n-----END PGP SIGNATURE-----\\n?$", Qt::CaseSensitive, QRegExp::RegExp2);
        if ( !regexp.exactMatch(armored) ) {
            throw toError("Invalid signature data");
        }

        const QByteArray crc = QByteArray::fromBase64(regexp.cap(2).toUtf8());
        if ( !crc24(crc) ) {
            throw toError("Invalid signature checksum");
        }

        return QByteArray::fromBase64(regexp.cap(1).toUtf8());
    }

    const QByteArray SignatureRSA::extractSignature(const QByteArray& raw)
    {
        if ( raw.size() < 5 ) {
            throw toError("Invalid raw data");
        }

        if ( !(raw.at(0) & 0b10000000) ) { // 1st bit set = new version
            throw toError("Old PGP packet format not supported");
        }

        char tagByte = raw.at(0) & 0b00011111; // take last 5 bits for tag value
        if ( tagByte != 2 ) { // signature
            throw toError("Not a signature");
        }

        fPacketLength = -1;
        int offset = -1;
        quint8 len1 = (quint8) raw.at(1);
        if ( len1 < 192 ) { // single octet length
            fPacketLength = len1;
            offset = 2; // tag + len byte
        } else if ( len1 >= 192 && len1 < 224 ) { // two octet length
            fPacketLength = ((len1 - 192) << 8) + ((quint8)raw.at(2)) + 192;
            offset = 3; // tag + two length bytes
        } else if ( len1 == 255 ) { // 5 octet length
            fPacketLength = ((quint8) raw.at(2) << 24) | ((quint8) raw.at(3) << 16) | ((quint8) raw.at(4) << 8) | (quint8) raw.at(5);
            offset = 6; // tag + five length bytes
        } else {
            throw toError("Invalid packet length encoding");
        }

        const QByteArray sigData = raw.mid(offset);
        if ( sigData.size() < 4 ) {
            throw toError("Invalid signature data size");
        }

        int index = 0;
        fVersion = sigData.at(index++);
        if ( fVersion != 4 ) { // v3 unsupported for detached!
            throw toError("Invalid signature version");
        }

        fType = sigData.at(index++);
        if ( fType != 0 ) { // 0 - binary
            throw toError("Signature type not supported");
        }

        fPubKeyType = sigData.at(index++);
        fHashType = sigData.at(index++);
        fHashedSize = ((quint8) sigData.at(index) << 8) + (quint8) sigData.at(index + 1); // do not use ++ here order is undefined!
        fHashedSubPackets = sigData.mid(index + 2, fHashedSize);
        index += (fHashedSize + 2);
        fUnhashedSize = ((quint8) sigData.at(index) << 8) + (quint8) sigData.at(index + 1); // do not use ++ here order is undefined!
        index += (fUnhashedSize + 2);
        index += 2; // skip the 16bits of the signed hashed value
        fSigNumSize = ((quint8) sigData.at(index) << 8) + (quint8) sigData.at(index + 1); // do not use ++ here order is undefined!
        index += 2;

        return sigData.mid(index);
    }

    bool SignatureRSA::crc24(const QByteArray& input) const
    {
        quint32 crc = CRC24_INIT;
        foreach ( char byte, input ) {
            crc ^= byte << 16;
            for (int i = 0; i < 8; i++) {
                crc <<= 1;
                if (crc & 0x1000000)
                    crc ^= CRC24_POLY;
            }
        }

        return crc & 0xFFFFFFL;
    }

    const QString SignatureRSA::toError(const QString& error) const
    {
        qDebug() << error << "\n";
        return error;
    }

}
