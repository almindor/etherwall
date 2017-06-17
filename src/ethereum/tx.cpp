#include "tx.h"
#include "helpers.h"
#include <QList>

namespace Ethereum {

    Tx::Tx()
    {

    }

    Tx::Tx(const QString &from, const QString &to,
           const QString &value, quint64 nonce,
           const QString &gas, const QString &gasPrice,
           const QString &data) :
        fFrom(from), fTo(to),
        fValue(value), fNonce(nonce),
        fGas(gas), fGasPrice(gasPrice),
        fData(data), fV(0), fR(), fS(),
        fDataIndex(0)
    {

    }

    void Tx::init(const QString &from, const QString &to, const QString &value, quint64 nonce, const QString &gas, const QString &gasPrice, const QString &data)
    {
        fFrom = from;
        fTo = to;
        fValue = value;
        fNonce = nonce;
        fGas = gas;
        fGasPrice = gasPrice;
        fData = data;
        fV = 0;
        fR = QByteArray();
        fS = QByteArray();
        fDataIndex = 0;
    }

    bool Tx::isContractDeploy() const
    {
        return fTo.isEmpty();
    }

    bool Tx::hasDefinedGas() const
    {
        return !fGas.isEmpty();
    }

    bool Tx::hasDefinedGasPrice() const
    {
        return !fGasPrice.isEmpty();
    }

    bool Tx::hasData() const
    {
        return !(fData.isEmpty() || fData == "0x" || fData == "0X");
    }

    const QString Tx::fromStr() const
    {
        return fFrom;
    }

    const QString Tx::toStr() const
    {
        return fTo;
    }

    const std::string Tx::toBytes() const
    {
        const QString hex = Etherwall::Helpers::clearHexPrefix(fTo);
        const QByteArray bytes = QByteArray::fromHex(hex.toUtf8());

        return bytes.toStdString();
    }

    bool Tx::hasValue() const
    {
        return Etherwall::Helpers::toHexWeiStr(fValue) != "0x0";
    }

    const QString Tx::valueStr() const
    {
        return fValue;
    }

    const QString Tx::valueHex() const
    {
        return Etherwall::Helpers::toHexWeiStr(fValue);
    }

    const std::string Tx::valueBytes() const
    {
        const QString hex = Etherwall::Helpers::clearHexPrefix(Etherwall::Helpers::toHexWeiStr(fValue));
        const QByteArray bytes = QByteArray::fromHex(hex.toUtf8());

        return bytes.toStdString();
    }

    const QString Tx::gasStr() const
    {
        return fGas;
    }

    const QString Tx::gasHex() const
    {
        return Etherwall::Helpers::decStrToHexStr(fGas);
    }

    const std::string Tx::gasBytes() const
    {
        const QString hex = Etherwall::Helpers::clearHexPrefix(Etherwall::Helpers::decStrToHexStr(fGas));
        const QByteArray bytes = QByteArray::fromHex(hex.toUtf8());

        return bytes.toStdString();
    }

    const QString Tx::gasPriceStr() const
    {
        return fGasPrice;
    }

    const QString Tx::gasPriceHex() const
    {
        return Etherwall::Helpers::toHexWeiStr(fGasPrice);
    }

    const std::string Tx::gasPriceBytes() const
    {
        const QString hex = Etherwall::Helpers::clearHexPrefix(Etherwall::Helpers::toHexWeiStr(fGasPrice));
        const QByteArray bytes = QByteArray::fromHex(hex.toUtf8());

        return bytes.toStdString();
    }

    quint64 Tx::nonce() const
    {
        return fNonce;
    }

    const std::string Tx::nonceBytes() const
    {
        const QString nonceHex = Etherwall::Helpers::clearHexPrefix(Etherwall::Helpers::toHexStr(fNonce)); // big endian hex string
        const QByteArray nonceBytes = QByteArray::fromHex(nonceHex.toUtf8());

        return nonceBytes.toStdString();
    }

    const QString Tx::nonceHex() const
    {
        return Etherwall::Helpers::toHexStr(fNonce);
    }

    quint32 Tx::dataByteSize() const
    {
        if ( !hasData() ) {
            return 0;
        }

        return QByteArray::fromHex(Etherwall::Helpers::clearHexPrefix(fData).toUtf8()).size();
    }

    const QString Tx::dataStr() const
    {
        return fData;
    }

    const QString Tx::dataHex() const
    {
        return Etherwall::Helpers::hexPrefix(fData);
    }

    const std::string Tx::dataBytes(quint32 index, quint16 size) const
    {
        const QByteArray allBytes = QByteArray::fromHex(Etherwall::Helpers::clearHexPrefix(fData).toUtf8());
        if ( index >= (quint32) allBytes.size() ) {
            return std::string();
        }

        return allBytes.mid(index, size).toStdString();
    }

    const std::string Tx::dataNext(quint16 size)
    {
        if ( size > dataByteSize() ) {
            size = dataByteSize();
        }

        const std::string result = dataBytes(fDataIndex, size);
        fDataIndex += size;
        return result;
    }

    void Tx::sign(quint32 v, const std::string &r, const std::string &s)
    {
        fV = v;
        fR = QByteArray::fromStdString(r);
        fS = QByteArray::fromStdString(s);
    }

    const std::string Tx::vBytes() const
    {
        const QString hex = Etherwall::Helpers::clearHexPrefix(Etherwall::Helpers::toHexStr(fV)); // big endian hex string
        const QByteArray bytes = QByteArray::fromHex(hex.toUtf8());

        return bytes.toStdString();
    }

    const std::string Tx::rBytes() const
    {
        return fR.toStdString();
    }

    const std::string Tx::sBytes() const
    {
        return fS.toStdString();
    }

    const QString Tx::encodeRLP(bool withSignature) const
    {
        QList<QByteArray> innerRLPs;
        innerRLPs.append(encodeRLPString(nonceBytes())); // nonce
        innerRLPs.append(encodeRLPString(gasPriceBytes())); // gas price
        innerRLPs.append(encodeRLPString(gasBytes())); // gas limit
        innerRLPs.append(encodeRLPString(toBytes())); // to addr
        innerRLPs.append(encodeRLPString(valueBytes())); // value
        innerRLPs.append(encodeRLPString(dataBytes(0, dataByteSize()))); // data

        if ( withSignature ) {
            innerRLPs.append(encodeRLPString(vBytes())); // signature/v
            innerRLPs.append(encodeRLPString(rBytes())); // signature/r
            innerRLPs.append(encodeRLPString(sBytes())); // signature/s
        }

        int totalSize = 0;
        foreach ( const QByteArray item, innerRLPs ) {
            totalSize += item.size();
        }

        QByteArray result;
        foreach ( const QByteArray item, innerRLPs ) {
            result += item;
        }

        if ( totalSize <= 55 ) {
            result.prepend(0xc0 + totalSize);

            return result.toHex();
        }

        QByteArray binLength = lengthToBinary(totalSize);
        binLength.prepend(0xf7 + binLength.size());
        return (binLength + result).toHex();
    }

    const QString Tx::toString() const
    {
        return "from: " + fFrom + " to: " + fTo + " value: " + fValue + " nonce: " + fNonce + " gas: " + fGas + " gasPrice: " + fGasPrice + " data: " + fData + " v: " + fV + " r: " + fR.toHex() + " s: " + fS.toHex();
    }

    const QByteArray Tx::encodeRLPString(const std::string &bytes) const
    {
        QByteArray native = QByteArray::fromStdString(bytes);

        if ( native.size() == 1 && (quint8) native.at(0) <= 0x7f ) { // single byte case
            // 0 is not 0x00 because of https://forum.ethereum.org/discussion/1657/why-should-rlp-0-be-0x80
            if ( native.at(0) == 0 ) {
                native[0] = 0x80;
            }
            return native;
        }

        if ( native.size() <= 55 ) {
            quint8 size = native.size() + 0x80;
            native.prepend(size);
            return native;
        }

        // full > 55 bytes size
        QByteArray binLength = lengthToBinary(native.size());
        binLength.prepend(0xb7 + binLength.size());
        return binLength + native;
    }

    const QByteArray Tx::lengthToBinary(quint32 length) const
    {
        if ( length == 0 ) {
            return QByteArray();
        } else {
            return lengthToBinary(length / 256) + char(length % 256);
        }
    }

}
