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
#include <QSettings>
#include <QDateTime>

namespace Etherwall {

// ***************************** TransactionInfo ***************************** //

    AccountInfo::AccountInfo(const QString& hash, const QString& balance, quint64 transCount) :
        fHash(hash), fBalance(balance), fTransCount(transCount)
    {
    }

    const QVariant AccountInfo::value(const int role) const {
        switch ( role ) {
        case HashRole: return QVariant(fHash);
        case BalanceRole: return QVariant(fBalance.toDouble());
        case TransCountRole: return QVariant(fTransCount);
        case LockedRole: return QVariant(QSettings().value("accounts/" + fHash, 0).toLongLong() < QDateTime::currentMSecsSinceEpoch());
        case SummaryRole: return QVariant(fHash + (fBalance.toDouble() > 0 ? " [> 0 Ether]" : " [empty]") );
        }

        return QVariant();
    }

    void AccountInfo::setBalance(const QString& balance) {
        fBalance = balance;
    }

    void AccountInfo::setTransactionCount(quint64 count) {
        fTransCount = count;
    }

    void AccountInfo::unlock(qint64 toTime) {
        if ( fHash.length() > 0 ) {
            QSettings settings;
            settings.setValue("accounts/" + fHash, toTime);
        }
    }

// ***************************** TransactionInfo ***************************** //

    TransactionInfo::TransactionInfo() {
        fValue = "0x0";
        fBlockNumber = 0;
        fTransactionIndex = 0;
    }

    TransactionInfo::TransactionInfo(const QJsonObject& source)
    {
        fHash = source.value("hash").toString("invalid");
        fNonce = Helpers::toQUInt64(source.value("nonce"));
        fSender = source.value("from").toString("invalid");
        fReceiver = source.value("to").toString("invalid");
        fBlockHash = source.value("blockHash").toString("invalid");
        fBlockNumber = Helpers::toQUInt64(source.value("blockNumber"));
        fTransactionIndex = Helpers::toQUInt64(source.value("transactionIndex"));
        fValue = Helpers::toDecStrEther(source.value("value"));
        fGas = Helpers::toDecStr(source.value("gas"));
        fGasPrice = Helpers::toDecStr(source.value("gasPrice"));
        fInput = source.value("gasPrice").toString("invalid");

    }

    const QVariant TransactionInfo::value(const int role) const {
        switch ( role ) {
            case THashRole: return QVariant(fHash);
            case NonceRole: return QVariant(fNonce);
            case SenderRole: return QVariant(fSender);
            case ReceiverRole: return QVariant(fReceiver);
            case ValueRole: return QVariant(fValue.toDouble());
            case BlockNumberRole: return QVariant(fBlockNumber);
            case BlockHashRole: return QVariant(fBlockHash);
            case TransactionIndexRole: return QVariant(fTransactionIndex);
            case GasRole: return QVariant(fGas);
            case GasPriceRole: return QVariant(fGasPrice);
            case InputRole: return QVariant(fInput);
        }

        return QVariant();
    }

    void TransactionInfo::setBlockNumber(quint64 num) {
        fBlockNumber = num;
    }

    void TransactionInfo::setHash(const QString& hash) {
        fHash = hash;
    }

    void TransactionInfo::init(const QString& from, const QString& to, double value, double gas) {
        fSender = from;
        fReceiver = to;
        fValue = QString::number(value);
        if ( gas > 0 ) {
            fGas = gas;
        }
    }

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

    const QString Helpers::toHexWeiStr(double val) {
        BigInt::Vin vinVal = BigInt::Vin::fromDouble(val * 1000000000000000000);
        return QString(vinVal.toStr0xHex().data());
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

}
