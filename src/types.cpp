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
/** @file types.cpp
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Types implementation
 */

#include "types.h"
#include "helpers.h"
#include <QSettings>
#include <QDateTime>
#include <QTimer>
#include <QJsonDocument>
#include <QDir>
#include <QDebug>

namespace Etherwall {

// ***************************** LogInfo ***************************** //

    LogInfo::LogInfo(const QString& info, LogSeverity sev) : fMsg(info), fDate(QDateTime::currentDateTime()), fSeverity(sev)
    {

    }

    const QVariant LogInfo::value(int role) const {
        switch ( role ) {
            case MsgRole: return QVariant(fMsg);
            case DateRole: return QVariant(fDate);
            case SeverityRole: return QVariant(getSeverityString());
        }

        return QVariant();
    }

    const QString LogInfo::getSeverityString() const {
        switch ( fSeverity ) {
            case LS_Debug: return QString("Debug");
            case LS_Info: return QString("Info");
            case LS_Warning: return QString("Warning");
            case LS_Error: return QString("Error");
        }

        return QString("Unknown");
    }

    // ***************************** Denomination ***************************** //

    CurrencyInfo::CurrencyInfo( const QString name, const float price ) : fName(name), fPrice(price) {
    }

    const QVariant CurrencyInfo::value(const int role) const {
        switch ( role ) {
        case NameRole: return QVariant(fName);
        case PriceRole: return QVariant(fPrice);
        }

        return QVariant();
    }

    double CurrencyInfo::recalculate(const double ether) const {
        return ether * fPrice;
    }

    const QString CurrencyInfo::name() const
    {
        return fName;
    }


// ***************************** AccountInfo ***************************** //

    AccountInfo::AccountInfo(const QString &hash, const QString &alias, const QString &deviceID,
                             const QString &balance, quint64 transCount, const QString& hdPath, int network) :
         fHash(Helpers::vitalizeAddress(hash)), fAlias(alias), fDeviceID(deviceID),
         fBalance(balance), fTransCount(transCount), fHDPath(hdPath), fNetwork(network),
         fTokenBalances(), fCurrentTokenAddress()
    {
        // old alias compatibility
        if ( alias.isEmpty() ) {
            QSettings settings;
            const QString lowerHash = hash.toLower();

            if ( settings.contains("alias/" + lowerHash) ) {
                fAlias = settings.value("alias/" + lowerHash, QString()).toString();
                settings.remove("alias/" + lowerHash);
            }
        }
    }

    const QVariant AccountInfo::value(const int role) const {
        const QSettings settings;
        const QString defaultKey = "accounts/default/" + Helpers::networkPostfix(fNetwork);
        const QString defaultAccount = settings.value(defaultKey).toString();

        switch ( role ) {
            case HashRole: return QVariant(fHash);
            case DefaultRole: return QVariant(fHash.toLower() == defaultAccount ? "✓" : "");
            case BalanceRole: return getBalance();
            case TransCountRole: return QVariant(fTransCount);
            case SummaryRole: return QVariant(getSummary());
            case AliasRole: return QVariant(fAlias.isEmpty() ? fHash : fAlias);
            case DeviceRole: return QVariant(fDeviceID);
            case DeviceTypeRole: return QVariant(fDeviceID == "geth" ? "" : "⊡");
            case HDPathRole: return QVariant(fHDPath);
            case TokenBalanceRole: return fTokenBalances.value(fCurrentTokenAddress);
        }

        return QVariant();
    }

    void AccountInfo::setBalance(const QString& balance) {
        fBalance = balance;
    }

    void AccountInfo::setTokenBalance(const QString& tokenAddress, const QString &balance)
    {
        fTokenBalances[tokenAddress] = balance;
    }

    void AccountInfo::setCurrentToken(const QString &tokenAddress)
    {
        fCurrentTokenAddress = tokenAddress;
    }

    void AccountInfo::setTransactionCount(quint64 count) {
        fTransCount = count;
    }

    const QString AccountInfo::getCurrentTokenAddress() const
    {
        return fCurrentTokenAddress;
    }

    const QString AccountInfo::getTokenBalance(const QString &tokenAddress) const
    {
        return fTokenBalances.value(tokenAddress, "0");
    }

    bool AccountInfo::isLocal() const
    {
        return fHDPath.isEmpty();
    }

    void AccountInfo::setDeviceID(const QString &deviceID)
    {
        fDeviceID = deviceID;
    }

    const QString AccountInfo::deviceID() const
    {
        return fDeviceID;
    }

    void AccountInfo::setAlias(const QString& name) {
        fAlias = name;
    }

    const QString AccountInfo::alias() const
    {
        return fAlias;
    }

    const QString AccountInfo::hash() const
    {
        return fHash;
    }

    quint64 AccountInfo::transactionCount() const
    {
        return fTransCount;
    }

    const QJsonObject AccountInfo::toJson() const
    {
        QJsonObject result;
        result["hash"] = fHash.toLower();
        result["alias"] = fAlias;
        result["deviceID"] = fDeviceID;
        result["HDPath"] = fHDPath;

        return result;
    }

    const QString AccountInfo::HDPath() const
    {
        return fHDPath;
    }

    const QString AccountInfo::getSummary() const
    {
        return (fHDPath.isEmpty() ? "   " : "⊡ ") +  value(AliasRole).toString() + " [" + fBalance + "]";
    }

    const QString AccountInfo::getBalance() const
    {
        if ( fCurrentTokenAddress.isEmpty() ) {
            return fBalance;
        }

        return fTokenBalances.value(fCurrentTokenAddress);
    }

// ***************************** TransactionInfo ***************************** //

    TransactionInfo::TransactionInfo() : fSenderAlias(), fReceiverAlias()
    {
        fValue = "0x0";
        fBlockNumber = 0;
        fTransactionIndex = 0;
    }

    TransactionInfo::TransactionInfo(const TransactionInfo &other)
    {
        init(other.getSender(), other.getReceiver(), other.getValue(), other.getGas(), other.getGasPrice(), other.getData());

        fHash = other.getHash();
        fNonce = other.getNonce();
        fBlockNumber = other.getBlockNumber();
        fSenderAlias = other.getSenderAlias();
        fReceiverAlias = other.getReceiverAlias();
        fBlockHash = other.getBlockHash();
    }

    TransactionInfo::TransactionInfo(const QJsonObject& source) : fSenderAlias(), fReceiverAlias()
    {
        init(source);
    }

    TransactionInfo::TransactionInfo(const QString& hash, quint64 blockNum) : fHash(hash), fBlockNumber(blockNum), fSenderAlias(), fReceiverAlias()
    {
    }

    TransactionInfo::~TransactionInfo()
    {
    }

    const QVariant TransactionInfo::value(const int role) const {
        switch ( role ) {
            case THashRole: return QVariant(fHash);
            case NonceRole: return QVariant(fNonce);
            case SenderRole: return QVariant(fSender);
            case ReceiverRole: return QVariant(fReceiver);
            case ValueRole: return QVariant(fValue);
            case BlockNumberRole: return QVariant(fBlockNumber);
            case BlockHashRole: return QVariant(fBlockHash);
            case TransactionIndexRole: return QVariant(fTransactionIndex);
            case GasRole: return QVariant(fGas);
            case GasPriceRole: return QVariant(fGasPrice);
            case InputRole: return QVariant(fInput);
            case SenderAliasRole: return QVariant(fSenderAlias.isEmpty() ? fSender : fSenderAlias);
            case ReceiverAliasRole: return QVariant(fReceiverAlias.isEmpty() ? fReceiver : fReceiverAlias);
        }

        return QVariant();
    }

    const QString TransactionInfo::getValue() const
    {
        return fValue;
    }

    quint64 TransactionInfo::getBlockNumber() const {
        return fBlockNumber;
    }

    const QString TransactionInfo::getBlockHash() const
    {
        return fBlockHash;
    }

    void TransactionInfo::setBlockNumber(quint64 num) {
        fBlockNumber = num;
    }

    const QString TransactionInfo::getHash() const {
        return fHash;
    }

    void TransactionInfo::setHash(const QString& hash) {
        fHash = hash;
    }

    const QString TransactionInfo::getSender() const
    {
        return fSender;
    }

    const QString TransactionInfo::getReceiver() const
    {
        return fReceiver;
    }

    const QString TransactionInfo::getGas() const
    {
        return fGas;
    }

    const QString TransactionInfo::getGasPrice() const
    {
        return fGasPrice;
    }

    const QString TransactionInfo::getData() const
    {
        return fInput;
    }

    quint64 TransactionInfo::getNonce() const
    {
        return fNonce;
    }

    const QString TransactionInfo::getSenderAlias() const
    {
        return fSenderAlias;
    }

    const QString TransactionInfo::getReceiverAlias() const
    {
        return fReceiverAlias;
    }

    void TransactionInfo::setSenderAlias(const QString &alias)
    {
        fSenderAlias = alias;
    }

    void TransactionInfo::setReceiverAlias(const QString &alias)
    {
        fReceiverAlias = alias;
    }

    void TransactionInfo::init(const QString& from, const QString& to, const QString& value, const QString& gas, const QString& gasPrice, const QString& data) {
        fSender = Helpers::vitalizeAddress(from);
        fReceiver = Helpers::vitalizeAddress(to);
        fNonce = 0;
        fValue = Helpers::formatEtherStr(value);
        if ( !gas.isEmpty() ) {
            fGas = gas;
        }
        if ( !gasPrice.isEmpty() ) {
            fGasPrice = gasPrice;
        }
        if ( !data.isEmpty() ) {
            fInput = data;
        }
    }

    void TransactionInfo::init(const QJsonObject source) {
        fHash = source.value("hash").toString("invalid");
        fSender = Helpers::vitalizeAddress(source.value("from").toString("invalid"));
        fReceiver = Helpers::vitalizeAddress(source.value("to").toString());
        fInput = source.value("input").toString();
        fNonce = Helpers::toQUInt64(source.value("nonce"));
        fBlockHash = source.value("blockHash").toString("invalid");
        fBlockNumber = Helpers::toQUInt64(source.value("blockNumber"));
        fTransactionIndex = Helpers::toQUInt64(source.value("transactionIndex"));
        fValue = Helpers::toDecStrEther(source.value("value"));
        fGas = Helpers::toDecStr(source.value("gas"));
        fGasPrice = Helpers::toDecStrEther(source.value("gasPrice"));
    }

    const QJsonObject TransactionInfo::toJson(bool decimal) const {
        QJsonObject result;
        result["hash"] = fHash;
        result["from"] = fSender;
        result["to"] = fReceiver;
        result["blockHash"] = fBlockHash;
        result["input"] = fInput;

        if ( decimal ) {
            result["value"] = fValue;
            result["gas"] = fGas;
            result["gasPrice"] = fGasPrice;
            result["blockNumber"] = (qint64) fBlockNumber;
            result["transactionIndex"] = (qint64) fTransactionIndex;
            result["nonce"] = (qint64) fNonce;
        } else { // hex
            result["value"] = Helpers::toHexWeiStr(fValue);
            result["gas"] = Helpers::decStrToHexStr(fGas);
            result["gasPrice"] = Helpers::toHexWeiStr(fGasPrice);
            result["blockNumber"] = Helpers::toHexStr(fBlockNumber);
            result["transactionIndex"] = Helpers::toHexStr(fTransactionIndex);
            result["nonce"] = Helpers::toHexStr(fNonce);
        }

        return result;
    }

    const QString TransactionInfo::toJsonString(bool decimal) const {
        const QJsonDocument doc(toJson(decimal));
        return doc.toJson(QJsonDocument::Compact);
    }

}

