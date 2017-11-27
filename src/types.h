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
/** @file types.h
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Types header
 */

#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QStandardPaths>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDateTime>
#include <QMetaType>

namespace Etherwall {

    enum LogRoles {
        MsgRole = Qt::UserRole + 1,
        DateRole,
        SeverityRole
    };

    enum LogSeverity {
        LS_Debug,
        LS_Info,
        LS_Warning,
        LS_Error
    };

    class LogInfo
    {
    public:
        LogInfo(const QString& info, LogSeverity sev);
        const QVariant value(int role) const;
    private:
        QString fMsg;
        QDateTime fDate;
        LogSeverity fSeverity;

        const QString getSeverityString() const;
    };

    typedef QList<LogInfo> LogList;

    enum CurrencyRoles {
        NameRole = Qt::UserRole + 1,
        PriceRole
    };

    class CurrencyInfo
    {
    public:
        CurrencyInfo( const QString name, const float price );
        const QVariant value(const int role) const;
        double recalculate(const double ether) const;
        const QString name() const;
    private:
        QString fName;
        float fPrice;
    };

    typedef QList<CurrencyInfo> CurrencyInfos;

    enum NodeRequestTypes {
        NoRequest,
        NewAccount,
        UnlockAccount,
        GetBlockNumber,
        GetAccountRefs,
        GetBalance,
        GetTransactionCount,
        GetPeerCount,
        SendTransaction,
        SignTransaction,
        Call,
        SendRawTransaction,
        GetGasPrice,
        EstimateGas,
        NewBlockFilter,
        NewEventFilter,
        GetFilterChanges,
        UninstallFilter,
        GetTransactionByHash,
        GetBlock,
        GetClientVersion,
        GetNetVersion,
        GetSyncing,
        GetLogs,
        GetTransactionReceipt
    };

    enum AccountRoles {
        HashRole = Qt::UserRole + 1,
        DefaultRole,
        BalanceRole,
        TransCountRole,
        SummaryRole,
        AliasRole,
        DeviceRole,
        DeviceTypeRole,
        HDPathRole,
        TokenBalanceRole
    };

    class AccountInfo
    {
    public:
        AccountInfo(const QString& hash, const QString& alias, const QString& deviceID,
                    const QString& balance, quint64 transCount, const QString& hdPath, const int network);

        const QVariant value(const int role) const;
        void setBalance(const QString& balance);
        void setTokenBalance(const QString& tokenAddress, const QString& balance);
        void setCurrentToken(const QString& tokenAddress);
        void setTransactionCount(quint64 count);
        const QString getCurrentTokenAddress() const;
        const QString getTokenBalance(const QString& tokenAddress) const;
        void lock();
        void unlock();
        bool isLocked() const;
        bool isLocal() const;
        void setDeviceID(const QString& deviceID);
        const QString deviceID() const;
        void setAlias(const QString& name);
        const QString alias() const;
        const QString hash() const;
        quint64 transactionCount() const;
        const QJsonObject toJson() const;
        const QString HDPath() const;
    private:
        QString fHash;
        QString fAlias;
        QString fDeviceID;
        QString fBalance; // in ether
        quint64 fTransCount;
        QString fHDPath;
        bool fLocked;
        int fNetwork;
        QMap<QString, QString> fTokenBalances; // token contract address -> balance
        QString fCurrentTokenAddress;

        const QString getSummary() const;
        const QString getBalance() const;
    };

    typedef QList<AccountInfo> AccountList;

    enum TransactionRoles {
        THashRole = Qt::UserRole + 1,
        NonceRole,
        SenderRole,
        ReceiverRole,
        ValueRole,
        BlockNumberRole,
        BlockHashRole,
        TransactionIndexRole,
        GasRole,
        GasPriceRole,
        InputRole,
        DepthRole,
        SenderAliasRole,
        ReceiverAliasRole
    };

    class TransactionInfo
    {
    public:
        TransactionInfo();
        TransactionInfo(const TransactionInfo& other);
        TransactionInfo(const QJsonObject& source);
        TransactionInfo(const QString& hash, quint64 blockNum); // for storing from server reply
        ~TransactionInfo();

        const QVariant value(const int role) const;
        const QString getValue() const;
        quint64 getBlockNumber() const;
        const QString getBlockHash() const;
        void setBlockNumber(quint64 num);
        const QString getHash() const;
        void setHash(const QString& hash);
        const QString getSender() const;
        const QString getReceiver() const;
        const QString getGas() const;
        const QString getGasPrice() const;
        const QString getData() const;
        quint64 getNonce() const;
        const QString getSenderAlias() const;
        const QString getReceiverAlias() const;
        void setSenderAlias(const QString& alias);
        void setReceiverAlias(const QString& alias);
        void init(const QString& from, const QString& to, const QString& value, const QString& gas = QString(),
                  const QString& gasPrice = QString(), const QString& data = QString());
        void init(const QJsonObject source);
        const QJsonObject toJson(bool decimal = false) const;
        const QString toJsonString(bool decimal = false) const;
    private:
        QString fHash;
        quint64 fNonce;
        QString fSender;
        QString fReceiver;
        QString fValue; // in ether
        quint64 fBlockNumber;
        QString fBlockHash;
        quint64 fTransactionIndex;
        QString fGas;
        QString fGasPrice;
        QString fInput;
        QString fSenderAlias;
        QString fReceiverAlias;
    };

    typedef QList<TransactionInfo> TransactionList;
}

#endif // TYPES_H
