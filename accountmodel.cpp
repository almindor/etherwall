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

    AccountModel::AccountModel(QObject *parent) :
        QAbstractListModel(parent), fAccountList(), fIpc()
    {
        try {
            const QSettings settings;
            const QString path = settings.value("ipc/path", DefaultIPCPath).toString();
            fIpc.connect(path);
            refresh();
        } catch ( std::exception ) {
            qDebug() << fIpc.getError() << "\n";
        }
    }

    QHash<int, QByteArray> AccountModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[HashRole] = "hash";
        roles[BalanceRole] = "balance";
        roles[TransCountRole] = "transactions";
        return roles;
    }

    int AccountModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const {
        return fAccountList.length();
    }

    QVariant AccountModel::data(const QModelIndex & index, int role) const {
        const int row = index.row();

        return fAccountList.at(row).value(role);
    }

    const QString AccountModel::newAccount(const QString& pw) {
        try {
            const int index = fAccountList.size();
            const QString acct = fIpc.newAccount(pw);
            if ( !acct.isEmpty() ) {
                beginInsertRows(QModelIndex(), index, index);
                refresh();
                endInsertRows();
                return acct;
            }
            return "Account creation failed: " + fIpc.getError();
        } catch ( std::exception ) {
            qDebug() << fIpc.getError() << "\n";
            return fIpc.getError();
        }
    }

    bool AccountModel::deleteAccount(int index, const QString& pw) {
        if ( index >= 0 && index < fAccountList.size() ) try {
            const QString hash = fAccountList.at(index).value(HashRole).toString();
            if ( fIpc.deleteAccount(hash, pw) ) {
                beginRemoveRows(QModelIndex(), index, index);
                refresh();
                endRemoveRows();
                return true;
            }

            return false;
        } catch ( std::exception ) {
            qDebug() << fIpc.getError() << "\n";
            return false;
        }

        return false;
    }

    void AccountModel::refresh() {
        try {
            QJsonArray accRefs = fIpc.getAccountRefs();
            fAccountList.clear(); // after the call if we exceptioned out!

            foreach( QJsonValue r, accRefs ) {
                const QString hash = r.toString("INVALID");
                const QString balance = fIpc.getBalance(r);
                const quint64 transCount = fIpc.getTransactionCount(r);

                fAccountList.append(AccountInfo(hash, balance, transCount));
            }
        } catch ( std::exception ) {
            qDebug() << fIpc.getError() << "\n";
            return;
        }
    }

}
