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
/** @file accountmodel.h
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Account model header
 */

#ifndef ACCOUNTMODEL_H
#define ACCOUNTMODEL_H

#include <QAbstractListModel>
#include "types.h"
#include "etheripc.h"

namespace Etherwall {

    class AccountModel : public QAbstractListModel
    {
        Q_OBJECT
        Q_PROPERTY(QString errorIPC MEMBER fError NOTIFY errorChangedIPC STORED false FINAL)
    public:
        AccountModel(const EtherIPC& ipc);
        QString getError() const;
        QHash<int, QByteArray> roleNames() const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

        Q_INVOKABLE void newAccount(const QString& pw);
        Q_INVOKABLE void deleteAccount(const QString& pw, int index);
    public slots:
        void connectToServerDone();
        void getAccountsDone(const AccountList& list);
        void newAccountDone(const QString& hash, int index);
        void deleteAccountDone(bool result, int index);
        void error(const QString& error, int code);
    signals:
        void connectToServerIPC(const QString& path);
        void getAccountsIPC();
        void newAccountIPC(const QString& password, int index);
        void deleteAccountIPC(const QString& hash, const QString& password, int index);
        void errorChangedIPC(const QString& error);
    private:
        AccountList fAccountList;
        QString fError;

        void refresh();
    };

}

#endif // ACCOUNTMODEL_H
