#ifndef TX_H
#define TX_H

#include <QString>
#include <QByteArray>

namespace Ethereum {

    class Tx
    {
    public:
        Tx();
        // units are ether strings here, not wei
        Tx(const QString& from, const QString& to,
           const QString& value, quint64 nonce,
           const QString& gas, const QString& gasPrice,
           const QString& data = QString());

        void init(const QString& from, const QString& to,
           const QString& value, quint64 nonce,
           const QString& gas, const QString& gasPrice,
           const QString& data = QString());

        bool hasFrom() const;
        bool hasTo() const;
        bool hasValue() const;
        bool hasDefinedGas() const;
        bool hasDefinedGasPrice() const;
        bool hasData() const;

        const QString fromStr() const;

        const QString toStr() const;
        const std::string toBytes() const;

        const QString valueStr() const;
        const QString valueHex() const;
        const std::string valueBytes() const;

        const QString gasStr() const;
        const QString gasHex() const;
        const std::string gasBytes() const;

        const QString gasPriceStr() const;
        const QString gasPriceHex() const;
        const std::string gasPriceBytes() const;

        quint64 nonce() const;
        const std::string nonceBytes() const;
        const QString nonceHex() const;

        quint32 dataByteSize() const;
        const QString dataStr() const;
        const QString dataHex() const;
        const std::string dataBytes(quint32 index, quint16 size) const;
        const std::string dataNext(quint16 size);

        void sign(quint32 v, const std::string& r, const std::string& s);
        const std::string vBytes() const;
        const std::string rBytes() const;
        const std::string sBytes() const;

        const QString encodeRLP(bool withSignature = true) const;
        const QString toString() const;
    private:
        QString fFrom;
        QString fTo;
        QString fValue;
        quint64 fNonce;
        QString fGas;
        QString fGasPrice;
        QString fData;
        quint32 fV;
        QByteArray fR;
        QByteArray fS;
        int fDataIndex;

        const QByteArray encodeRLPString(const std::string& bytes) const;
        const QByteArray lengthToBinary(quint32 length) const;
    };

}

#endif // TX_H
