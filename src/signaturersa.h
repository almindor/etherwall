#ifndef SIGNATURERSA_H
#define SIGNATURERSA_H

#include <QByteArray>
#include <QStringList>

namespace PGP {

    class SignatureRSA
    {
    public:
        SignatureRSA(const QString& filePath);
        SignatureRSA(const QByteArray& armored);
        const QByteArray hex() const;
        const char* raw() const;
        int rawSize() const;
        const QByteArray ending() const; // openPGP "append before hashing" section needed to get valid hash of data for signature check
    private:
        QStringList fComments;
        QByteArray fSignatureData;
        QByteArray fHashedSubPackets;
        int fPacketLength;
        quint8 fVersion;
        quint8 fType;
        quint8 fPubKeyType;
        quint8 fHashType;
        quint16 fHashedSize;
        quint16 fUnhashedSize;
        quint16 fSigNumSize;

        void read(const QByteArray& armored);
        const QByteArray getRawData(const QByteArray& armored) const;
        const QByteArray extractSignature(const QByteArray& raw);
        bool crc24(const QByteArray& crc) const;
        const QString toError(const QString& error) const;
    };

}

#endif // SIGNATURERSA_H
