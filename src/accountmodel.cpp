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
#include <QJsonArray>
#include <QJsonValue>
#include <QSettings>

namespace Etherwall {

    AccountModel::AccountModel(EtherIPC& ipc) :
        QAbstractListModel(0), fIpc(ipc), fAccountList()
    {
        connect(&ipc, &EtherIPC::connectToServerDone, this, &AccountModel::connectToServerDone);
        connect(&ipc, &EtherIPC::getAccountsDone, this, &AccountModel::getAccountsDone);
        connect(&ipc, &EtherIPC::newAccountDone, this, &AccountModel::newAccountDone);
        connect(&ipc, &EtherIPC::deleteAccountDone, this, &AccountModel::deleteAccountDone);
    }

    QHash<int, QByteArray> AccountModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[HashRole] = "hash";
        roles[BalanceRole] = "balance";
        roles[TransCountRole] = "transactions";
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

    void AccountModel::connectToServerDone() {
        fIpc.getAccounts();
    }

    void AccountModel::getAccountsDone(const AccountList &list) {
        beginResetModel();
        fAccountList = list;
        endResetModel();
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

}
