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
/** @file accountmodel.cpp
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Account model implementation
 */

#include "accountmodel.h"
#include "types.h"
#include <QDebug>
#include <QSettings>
#include <QDateTime>

namespace Etherwall {

    AccountModel::AccountModel(EtherIPC& ipc, const CurrencyModel& currencyModel) :
        QAbstractListModel(0), fIpc(ipc), fAccountList(), fSelectedAccountRow(-1), fCurrencyModel(currencyModel)
    {
        connect(&ipc, &EtherIPC::connectToServerDone, this, &AccountModel::connectToServerDone);
        connect(&ipc, &EtherIPC::getAccountsDone, this, &AccountModel::getAccountsDone);
        connect(&ipc, &EtherIPC::newAccountDone, this, &AccountModel::newAccountDone);
        connect(&ipc, &EtherIPC::deleteAccountDone, this, &AccountModel::deleteAccountDone);
        connect(&ipc, &EtherIPC::unlockAccountDone, this, &AccountModel::unlockAccountDone);
        connect(&ipc, &EtherIPC::accountChanged, this, &AccountModel::accountChanged);
        connect(&ipc, &EtherIPC::newBlock, this, &AccountModel::newBlock);

        connect(&currencyModel, &CurrencyModel::currencyChanged, this, &AccountModel::currencyChanged);
    }

    QHash<int, QByteArray> AccountModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[HashRole] = "hash";
        roles[BalanceRole] = "balance";
        roles[TransCountRole] = "transactions";
        roles[LockedRole] = "locked";
        roles[SummaryRole] = "summary";
        roles[AliasRole] = "alias";
        roles[IndexRole] = "index";

        return roles;
    }

    int AccountModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const {
        return fAccountList.size();
    }

    QVariant AccountModel::data(const QModelIndex & index, int role) const {
        const int row = index.row();

        QVariant result = fAccountList.at(row).value(role);
        if ( role == BalanceRole ) {
            return fCurrencyModel.recalculate(result);
        }

        return result;
    }

    // TODO: optimize with hashmap
    bool AccountModel::containsAccount(const QString& from, const QString& to, int& i1, int& i2) const {
        i1 = -1;
        i2 = -1;
        int i = 0;
        foreach ( const AccountInfo& a, fAccountList ) {
            const QString addr = a.value(HashRole).toString();
            if ( addr == from ) {
                i1 = i;
            }

            if ( addr == to ) {
                i2 = i;
            }
            i++;
        }

        return (i1 >= 0 || i2 >= 0);
    }

    const QString AccountModel::getTotal() const {
        BigInt::Rossi total;

        foreach ( const AccountInfo& info, fAccountList ) {
            const QVariant balance = info.value(BalanceRole);
            double dVal = fCurrencyModel.recalculate(balance).toDouble();
            const QString strVal = QString::number(dVal, 'f', 18);
            total += Helpers::etherStrToRossi(strVal);
        }

        const QString weiStr = QString(total.toStrDec().data());
        return Helpers::weiStrToEtherStr(weiStr);
    }

    void AccountModel::newAccount(const QString& pw) {
        const int index = fAccountList.size();
        fIpc.newAccount(pw, index);
    }

    void AccountModel::renameAccount(const QString& name, int index) {
        if ( index >= 0 && index < fAccountList.size() ) {
            fAccountList[index].alias(name);

            QVector<int> roles(2);
            roles[0] = AliasRole;
            roles[1] = SummaryRole;
            const QModelIndex& modelIndex = QAbstractListModel::createIndex(index, 0);

            emit dataChanged(modelIndex, modelIndex, roles);
        } else {
            EtherLog::logMsg("Invalid account selection for rename", LS_Error);
        }
    }

    void AccountModel::deleteAccount(const QString& pw, int index) {
        if ( index >= 0 && index < fAccountList.size() ) {            
            const QString hash = fAccountList.at(index).value(HashRole).toString();
            fIpc.deleteAccount(hash, pw, index);
        } else {
            EtherLog::logMsg("Invalid account selection for delete", LS_Error);
        }
    }

    void AccountModel::unlockAccount(const QString& pw, int duration, int index) {
        if ( index >= 0 && index < fAccountList.size() && duration > 0 ) {
            const QString hash = fAccountList.at(index).value(HashRole).toString();
            fIpc.unlockAccount(hash, pw, duration, index);
        } else {
            EtherLog::logMsg("Invalid account selection for unlock");
        }
    }

    bool AccountModel::isLocked(int index) const {
        if ( index < 0 || index >= fAccountList.length() ) {
            return true;
        }

        return fAccountList.at(index).isLocked();
    }

    const QString AccountModel::getAccountHash(int index) const {
        if ( index >= 0 && fAccountList.length() > index ) {
            return fAccountList.at(index).value(HashRole).toString();
        }

        return QString();
    }

    void AccountModel::connectToServerDone() {
        fIpc.getAccounts();
    }

    void AccountModel::newAccountDone(const QString& hash, int index) {
        if ( !hash.isEmpty() ) {
            beginInsertRows(QModelIndex(), index, index);
            fAccountList.append(AccountInfo(hash, "0.00000000000000000", 0));
            endInsertRows();
            EtherLog::logMsg("New account created");
        } else {
            EtherLog::logMsg("Account create failure");
        }
    }

    void AccountModel::deleteAccountDone(bool result, int index) {
        if ( result ) {
            beginRemoveRows(QModelIndex(), index, index);
            fAccountList.removeAt(index);
            endRemoveRows();
            EtherLog::logMsg("Account deleted");
        } else {
            EtherLog::logMsg("Account delete failure");
        }
    }

    void AccountModel::unlockAccountDone(bool result, int index) {
        if ( result ) {
            QSettings settings;
            qint64 diff = settings.value("/ipc/accounts/lockduration", 300).toInt() * 1000;

            QTimer::singleShot(diff + 200, this, SLOT(checkAccountLocks()));
            fAccountList[index].unlock(QDateTime::currentMSecsSinceEpoch() + diff);
            const QModelIndex& modelIndex = QAbstractListModel::createIndex(index, 0);
            emit dataChanged(modelIndex, modelIndex, QVector<int>(1, LockedRole));
            emit accountLockedChanged(index);

            EtherLog::logMsg("Account unlocked");
        } else {
            EtherLog::logMsg("Account unlock failure");
        }
    }

    void AccountModel::checkAccountLocks() {
        int index = 0;
        foreach ( AccountInfo i, fAccountList ) {
            if ( i.value(LockedRole).toBool() != i.isLocked(true) ) {
                i.lock();
                const QModelIndex& modelIndex = QAbstractListModel::createIndex(index, 0);
                emit dataChanged(modelIndex, modelIndex, QVector<int>(1, LockedRole));
                emit accountLockedChanged(index);
            }
            index++;
        }
    }

    void AccountModel::currencyChanged() {
        QVector<int> roles(1);
        roles[0] = BalanceRole;

        const QModelIndex& leftIndex = QAbstractListModel::createIndex(0, 0);
        const QModelIndex& rightIndex = QAbstractListModel::createIndex(fAccountList.size() - 1, 4);
        emit dataChanged(leftIndex, rightIndex, roles);
        emit totalChanged();
    }

    void AccountModel::getAccountsDone(const AccountList& list) {
        beginResetModel();
        fAccountList = list;
        endResetModel();

        int i = 0;
        foreach ( const AccountInfo& info, list ) {
            const QString& hash = info.value(HashRole).toString();
            fIpc.refreshAccount(hash, i++);
        }

        emit totalChanged();
    }

    void AccountModel::accountChanged(const AccountInfo& info) {
        int index = 0;
        const QString infoHash = info.value(HashRole).toString();
        foreach ( const AccountInfo& a, fAccountList ) {
            if ( a.value(HashRole).toString() == infoHash ) {
                fAccountList[index] = info;
                const QModelIndex& leftIndex = QAbstractListModel::createIndex(index, 0);
                const QModelIndex& rightIndex = QAbstractListModel::createIndex(index, 4);
                emit dataChanged(leftIndex, rightIndex);
                emit totalChanged();
                return;
            }
            index++;
        }

        const int len = fAccountList.length();
        beginInsertRows(QModelIndex(), len, len);
        fAccountList.append(info);
        endInsertRows();

        emit totalChanged();
    }

    void AccountModel::newBlock(const QJsonObject& block) {
        const QJsonArray transactions = block.value("transactions").toArray();
        const QString miner = block.value("miner").toString("bogus");
        int i1, i2;
        if ( containsAccount(miner, "bogus", i1, i2) ) {
            fIpc.refreshAccount(miner, i1);
        }

        foreach ( QJsonValue t, transactions ) {
            const QJsonObject to = t.toObject();
            const TransactionInfo info(to);
            const QString& sender = info.value(SenderRole).toString();
            const QString& receiver = info.value(ReceiverRole).toString();

            if ( containsAccount(sender, receiver, i1, i2) ) {
                if ( i1 >= 0 ) {
                    fIpc.refreshAccount(sender, i1);
                }

                if ( i2 >= 0 ) {
                    fIpc.refreshAccount(receiver, i2);
                }
            }
        }

        emit totalChanged();
    }

    int AccountModel::getSelectedAccountRow() const {
        return fSelectedAccountRow;
    }

    void AccountModel::setSelectedAccountRow(int row) {
        fSelectedAccountRow = row;
        emit accountSelectionChanged(row);
    }

    const QString AccountModel::getSelectedAccount() const {
        return getAccountHash(fSelectedAccountRow);
    }

    const QJsonArray AccountModel::getAccountsJsonArray() const {
        QJsonArray result;

        foreach ( const AccountInfo ai, fAccountList ) {
            const QString hash = ai.value(HashRole).toString();
            result.append(hash);
        }

        return result;
    }

}
