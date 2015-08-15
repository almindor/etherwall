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

namespace Etherwall {

#ifdef Q_OS_WIN32
    static const QString DefaultIPCPath = "//.pipe/geth.ipc";
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
        GetPeerCount
    };

    enum AccountRoles {
        HashRole = Qt::UserRole + 1,
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
    private:
        QString fHash;
        QString fBalance; // in ether
        quint64 fTransCount;
    };

    typedef QList<AccountInfo> AccountList;

    enum TransactionRoles {
        SenderRole = Qt::UserRole + 1,
        ReceiverRole,
        ValueRole,
        BlockNumberRole,
        BlockHashRole
    };

    class TransactionInfo
    {
    public:
        TransactionInfo(const QString& sender, const QString& receiver, const QString& value, quint64 blockNumber, const QString& blockHash);

        const QVariant value(const int role) const;
    private:
        QString fSender;
        QString fReceiver;
        QString fValue; // in ether
        quint64 fBlockNumber;
        QString fBlockHash;
    };

    typedef QList<TransactionInfo> TransactionList;

}

#endif // TYPES_H
