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

#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include <QtCore/qsortfilterproxymodel.h>
#include <QtQml/qjsvalue.h>

namespace Etherwall {

    class AccountProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT
        Q_PROPERTY(int count READ count NOTIFY countChanged)
        Q_PROPERTY(QObject *source READ source WRITE setSource)

        Q_PROPERTY(QByteArray sortRole READ sortRole WRITE setSortRole)
        Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder)

        Q_PROPERTY(QByteArray filterRole READ filterRole WRITE setFilterRole)
        Q_PROPERTY(QString filterString READ filterString WRITE setFilterString)
        Q_PROPERTY(FilterSyntax filterSyntax READ filterSyntax WRITE setFilterSyntax)

        Q_ENUMS(FilterSyntax)

    public:
        explicit AccountProxyModel(QObject *parent = 0);

        QObject *source() const;
        void setSource(QObject *source);

        QByteArray sortRole() const;
        void setSortRole(const QByteArray &role);

        void setSortOrder(Qt::SortOrder order);

        QByteArray filterRole() const;
        void setFilterRole(const QByteArray &role);

        QString filterString() const;
        void setFilterString(const QString &filter);

        enum FilterSyntax {
            RegExp,
            Wildcard,
            FixedString
        };

        FilterSyntax filterSyntax() const;
        void setFilterSyntax(FilterSyntax syntax);

        int count() const;
        Q_INVOKABLE QJSValue get(int index) const;

    signals:
        void countChanged();

    protected:
        int roleKey(const QByteArray &role) const;
        QHash<int, QByteArray> roleNames() const;
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    };

}

#endif // SORTFILTERPROXYMODEL_H
