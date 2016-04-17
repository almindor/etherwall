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
#include <QList>
#include <QVariant>
#include <QStandardPaths>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDateTime>
#include "bigint.h"

namespace Etherwall {

#ifdef Q_OS_WIN32
    static const QString DefaultDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Ethereum";
    static const QString DefaultIPCPath = "\\\\.\\pipe\\geth.ipc";
    static const QString DefaultGethPath = "./geth.exe";
#else
    #ifdef Q_OS_MACX
    static const QString DefaultDataDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Library/Ethereum";
    static const QString DefaultIPCPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Library/Ethereum/geth.ipc";
    static const QString DefaultGethPath = "./geth";
    #else
    static const QString DefaultDataDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.ethereum";
    static const QString DefaultIPCPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.ethereum/geth.ipc";
    static const QString DefaultGethPath = "/usr/bin/geth";
    #endif
#endif

    static const quint64 SYNC_DEPTH = 10;
    static const QString DefaultGethArgs = "--fast --cache 512";

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
        double recalculate(const float ether) const;
    private:
        QString fName;
        float fPrice;
    };

    typedef QList<CurrencyInfo> CurrencyInfos;

    enum RequestTypes {
        NoRequest,
        NewAccount,
        DeleteAccount,
        GetBlockNumber,
        GetAccountRefs,
        GetBalance,
        GetTransactionCount,
        GetPeerCount,
        SendTransaction,
        UnlockAccount,
        GetGasPrice,
        EstimateGas,
        NewFilter,
        GetFilterChanges,
        UninstallFilter,
        GetTransactionByHash,
        GetBlock,
        GetClientVersion,
        GetSyncing
    };

    enum AccountRoles {
        LockedRole = Qt::UserRole + 1,
        HashRole,
        BalanceRole,
        TransCountRole,
        SummaryRole,
        AliasRole,
        IndexRole
    };

    class AccountInfo
    {
    public:
        AccountInfo(const QString& hash, const QString& balance, quint64 transCount);

        const QVariant value(const int role) const;
        void setBalance(const QString& balance);
        void setTransactionCount(quint64 count);
        void lock();
        void unlock(qint64 toTime);
        bool isLocked(bool internal = false) const;
        void alias(const QString& name);
    private:
        int fIndex;
        QString fHash;
        QString fBalance; // in ether
        quint64 fTransCount;
        QString fAlias;
        bool fLocked;
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
        TransactionInfo(const QJsonObject& source);
        TransactionInfo(const QString& hash, quint64 blockNum); // for storing from server reply

        const QVariant value(const int role) const;
        quint64 getBlockNumber() const;
        void setBlockNumber(quint64 num);
        const QString getHash() const;
        void setHash(const QString& hash);
        void init(const QString& from, const QString& to, const QString& value, const QString& gas = QString());
        void init(const QJsonObject source);
        const QJsonObject toJson(bool decimal = false) const;
        const QString toJsonString(bool decimal = false) const;
        void lookupAccountAliases();
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
    };

}

#endif // TYPES_H
