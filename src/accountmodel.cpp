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

    AccountModel::AccountModel(EtherIPC& ipc) :
        QAbstractListModel(0), fIpc(ipc), fAccountList(), fSelectedAccountRow(-1)
    {
        connect(&ipc, &EtherIPC::connectToServerDone, this, &AccountModel::connectToServerDone);
        connect(&ipc, &EtherIPC::newAccountDone, this, &AccountModel::newAccountDone);
        connect(&ipc, &EtherIPC::deleteAccountDone, this, &AccountModel::deleteAccountDone);
        connect(&ipc, &EtherIPC::unlockAccountDone, this, &AccountModel::unlockAccountDone);
        connect(&ipc, &EtherIPC::accountChanged, this, &AccountModel::accountChanged);
        connect(&ipc, &EtherIPC::newBlock, this, &AccountModel::newBlock);
    }

    QHash<int, QByteArray> AccountModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[HashRole] = "hash";
        roles[BalanceRole] = "balance";
        roles[TransCountRole] = "transactions";
        roles[LockedRole] = "locked";
        roles[SummaryRole] = "summary";
        return roles;
    }

    int AccountModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const {
        return fAccountList.length();
    }

    QVariant AccountModel::data(const QModelIndex & index, int role) const {
        const int row = index.row();

        return fAccountList.at(row).value(role);
    }

    bool AccountModel::containsAccount(const TransactionInfo& info, int& ai1, int& ai2) const {
        int i = 0;
        ai1 = -1;
        ai2 = -1;

        foreach ( const AccountInfo& a, fAccountList ) {
            const QString& ahash = a.value(HashRole).toString();
            const QString& fhash = info.value(SenderRole).toString();
            if ( ahash == fhash ) {
                ai1 = i;
                break;
            }
            i++;
        }

        foreach ( const AccountInfo& a, fAccountList ) {
            const QString& ahash = a.value(HashRole).toString();
            const QString& thash = info.value(ReceiverRole).toString();
            if ( ahash == thash ) {
                ai2 = i;
                break;
            }
            i++;
        }

        return (ai1 > 0 || ai2 > 0);
    }

    void AccountModel::newAccount(const QString& pw) {
        const int index = fAccountList.size();
        fIpc.newAccount(pw, index);
    }

    void AccountModel::deleteAccount(const QString& pw, int index) {
        if ( index >= 0 && index < fAccountList.size() ) {            
            const QString hash = fAccountList.at(index).value(HashRole).toString();
            fIpc.deleteAccount(hash, pw, index);
        } else {
            qDebug() << "Invalid account selection for delete";
        }
    }

    void AccountModel::unlockAccount(const QString& pw, int duration, int index) {
        if ( index >= 0 && index < fAccountList.size() && duration > 0 ) {
            const QString hash = fAccountList.at(index).value(HashRole).toString();
            qDebug() << "model unlock";
            fIpc.unlockAccount(hash, pw, duration, index);
        } else {
            qDebug() << "Invalid account selection for unlock";
        }
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
            fAccountList.append(AccountInfo(hash, (QString("0") + QLocale().decimalPoint() + "00000000000000000"), 0));
            endInsertRows();
        } else {
            qDebug() << "Account create failure";
        }
    }

    void AccountModel::deleteAccountDone(bool result, int index) {
        if ( result ) {
            beginRemoveRows(QModelIndex(), index, index);
            fAccountList.removeAt(index);
            endRemoveRows();
        } else {
            qDebug() << "Account delete failure";
        }
    }

    void AccountModel::unlockAccountDone(bool result, int index) {
        if ( result ) {
            QSettings settings;
            qint64 diff = settings.value("/ipc/accounts/lockduration", 10).toInt() * 1000;

            fAccountList[index].unlock(QDateTime::currentMSecsSinceEpoch() + diff);
            const QModelIndex& modelIndex = QAbstractListModel::createIndex(index, 0);
            emit dataChanged(modelIndex, modelIndex, QVector<int>(1, LockedRole));
        } else {
            qDebug() << "Account unlock failure";
        }
    }

    void AccountModel::accountChanged(const AccountInfo& info) {
        int index = 0;
        foreach ( const AccountInfo& a, fAccountList ) {
            if ( a.value(HashRole).toString() == info.value(HashRole).toString() ) {
                fAccountList[index] = info;
                const QModelIndex& leftIndex = QAbstractListModel::createIndex(index, 0);
                const QModelIndex& rightIndex = QAbstractListModel::createIndex(index, 4);
                emit dataChanged(leftIndex, rightIndex);
                return;
            }
            index++;
        }

        const int len = fAccountList.length();
        beginInsertRows(QModelIndex(), len, len);
        fAccountList.append(info);
        endInsertRows();
    }

    void AccountModel::newBlock(const QJsonObject& block) {
        const QJsonArray transactions = block.value("transactions").toArray();

        foreach ( QJsonValue t, transactions ) {
            const QJsonObject to = t.toObject();
            const TransactionInfo info(to);
            int i1, i2;

            if ( containsAccount(info, i1, i2) ) {
                if ( i1 >= 0 ) {
                    fIpc.refreshAccount(info.value(SenderRole).toString(), i1);
                }

                if ( i2 >= 0 ) {
                    fIpc.refreshAccount(info.value(ReceiverRole).toString(), i2);
                }
            }
        }
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

}
