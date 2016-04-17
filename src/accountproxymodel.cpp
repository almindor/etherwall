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
/** @file accountproxymodel.h
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * Account proxy model header
 */

#include "accountproxymodel.h"
#include <QtDebug>
#include <QtQml>

namespace Etherwall {

    AccountProxyModel::AccountProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
    {
        connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SIGNAL(countChanged()));
        connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    }

    int AccountProxyModel::count() const
    {
        return rowCount();
    }

    QObject *AccountProxyModel::source() const
    {
        return sourceModel();
    }

    void AccountProxyModel::setSource(QObject *source)
    {
        setSourceModel(qobject_cast<QAbstractItemModel *>(source));
    }

    QByteArray AccountProxyModel::sortRole() const
    {
        return roleNames().value(QSortFilterProxyModel::sortRole());
    }

    void AccountProxyModel::setSortRole(const QByteArray &role)
    {
        QSortFilterProxyModel::setSortRole(roleKey(role));
    }

    void AccountProxyModel::setSortOrder(Qt::SortOrder order)
    {
        QSortFilterProxyModel::sort(0, order);
    }

    QByteArray AccountProxyModel::filterRole() const
    {
        return roleNames().value(QSortFilterProxyModel::filterRole());
    }

    void AccountProxyModel::setFilterRole(const QByteArray &role)
    {
        QSortFilterProxyModel::setFilterRole(roleKey(role));
    }

    QString AccountProxyModel::filterString() const
    {
        return filterRegExp().pattern();
    }

    void AccountProxyModel::setFilterString(const QString &filter)
    {
        setFilterRegExp(QRegExp(filter, filterCaseSensitivity(), static_cast<QRegExp::PatternSyntax>(filterSyntax())));
    }

    AccountProxyModel::FilterSyntax AccountProxyModel::filterSyntax() const
    {
        return static_cast<FilterSyntax>(filterRegExp().patternSyntax());
    }

    void AccountProxyModel::setFilterSyntax(AccountProxyModel::FilterSyntax syntax)
    {
        setFilterRegExp(QRegExp(filterString(), filterCaseSensitivity(), static_cast<QRegExp::PatternSyntax>(syntax)));
    }

    QJSValue AccountProxyModel::get(int idx) const
    {
        QJSEngine *engine = qmlEngine(this);
        QJSValue value = engine->newObject();
        if (idx >= 0 && idx < count()) {
            QHash<int, QByteArray> roles = roleNames();
            QHashIterator<int, QByteArray> it(roles);
            while (it.hasNext()) {
                it.next();
                value.setProperty(QString::fromUtf8(it.value()), data(index(idx, 0), it.key()).toString());
            }
        }
        return value;
    }

    int AccountProxyModel::roleKey(const QByteArray &role) const
    {
        QHash<int, QByteArray> roles = roleNames();
        QHashIterator<int, QByteArray> it(roles);
        while (it.hasNext()) {
            it.next();
            if (it.value() == role)
                return it.key();
        }
        return -1;
    }

    QHash<int, QByteArray> AccountProxyModel::roleNames() const
    {
        if (QAbstractItemModel *source = sourceModel())
            return source->roleNames();
        return QHash<int, QByteArray>();
    }

    bool AccountProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        QRegExp rx = filterRegExp();
        if (rx.isEmpty())
            return true;
        QAbstractItemModel *model = sourceModel();
        if (filterRole().isEmpty()) {
            QHash<int, QByteArray> roles = roleNames();
            QHashIterator<int, QByteArray> it(roles);
            while (it.hasNext()) {
                it.next();
                QModelIndex sourceIndex = model->index(sourceRow, 0, sourceParent);
                QString key = model->data(sourceIndex, it.key()).toString();
                if (key.contains(rx))
                    return true;
            }
            return false;
        }
        QModelIndex sourceIndex = model->index(sourceRow, 0, sourceParent);
        if (!sourceIndex.isValid())
            return true;
        QString key = model->data(sourceIndex, roleKey(filterRole())).toString();
        return key.contains(rx);
    }

}
