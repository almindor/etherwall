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
    public:
        explicit AccountModel(QObject *parent = 0);

        QHash<int, QByteArray> roleNames() const;

        int rowCount(const QModelIndex & parent = QModelIndex()) const;

        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

        Q_INVOKABLE const QString newAccount(const QString& pw);
        Q_INVOKABLE bool deleteAccount(int index, const QString& pw);
    private:
        AccountList fAccountList;
        EtherIPC fIpc;

        void refresh();
    signals:

    public slots:

    };

}

#endif // ACCOUNTMODEL_H
