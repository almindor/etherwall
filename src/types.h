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
#include "bigint.h"

namespace Etherwall {

#ifdef Q_OS_WIN32
    static const QString DefaultIPCPath = "\\\\.\\pipe\\geth.ipc";
#else
    #ifdef Q_OS_MACX
    static const QString DefaultIPCPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Library/Ethereum/geth.ipc";
    #else
    static const QString DefaultIPCPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.ethereum/geth.ipc";
    #endif
#endif

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
        GetClientVersion
    };

    enum AccountRoles {
        LockedRole = Qt::UserRole + 1,
        HashRole,
        BalanceRole,
        TransCountRole,
        SummaryRole
    };

    class AccountInfo
    {
    public:
        AccountInfo(const QString& hash, const QString& balance, quint64 transCount);

        const QVariant value(const int role) const;
        void setBalance(const QString& balance);
        void setTransactionCount(quint64 count);
        void unlock(qint64 toTime);
    private:
        QString fHash;
        QString fBalance; // in ether
        quint64 fTransCount;
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
        DepthRole
    };

    class TransactionInfo
    {
    public:
        TransactionInfo();
        TransactionInfo(const QJsonObject& source);

        const QVariant value(const int role) const;
        void setBlockNumber(quint64 num);
        void setHash(const QString& hash);
        void init(const QString& from, const QString& to, double value, double gas = -1.0);
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
    };

    typedef QList<TransactionInfo> TransactionList;

    class Helpers {
    public:
        static const QString toDecStr(const QJsonValue &jv);
        static const QString toDecStrEther(const QJsonValue &jv);
        static const QString toDecStr(quint64 val);
        static const QString toHexStr(quint64 val);
        static const QString toHexWeiStr(double val);
        static const QJsonArray toQJsonArray(const AccountList& list);
        static quint64 toQUInt64(const QJsonValue& jv);
    };

}

#endif // TYPES_H
