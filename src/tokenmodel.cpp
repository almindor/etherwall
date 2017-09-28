#include "tokenmodel.h"
#include "contractmodel.h"
#include <QDebug>

namespace Etherwall {

    TokenModel::TokenModel(QAbstractItemModel* source) :
        QAbstractListModel(0)
    {
        fFilteredContracts.setSourceModel(source);
        fFilteredContracts.setFilterRole(TokenRole);
        fFilteredContracts.setFilterRegExp(".+");

        connect(source, &QAbstractListModel::modelReset, this, &TokenModel::modelReset);
        connect(source, &QAbstractListModel::dataChanged, this, &TokenModel::onDataChanged);
        connect(source, &QAbstractListModel::rowsInserted, this, &TokenModel::onRowsChanged);
        connect(source, &QAbstractListModel::rowsRemoved, this, &TokenModel::onRowsChanged);
    }

    QHash<int, QByteArray> TokenModel::roleNames() const
    {
        QHash<int, QByteArray> roles;
        roles[TokenRole] = "token";

        return roles;
    }

    int TokenModel::rowCount(const QModelIndex &parent) const
    {
        return fFilteredContracts.rowCount(parent) + 1;
    }

    QVariant TokenModel::data(const QModelIndex &index, int role) const
    {
        if ( index.row() == 0 ) {
            return QVariant("ETH");
        }

        QModelIndex origIndex = fFilteredContracts.index(index.row(), index.column());
        if ( index.row() > 0 ) {
            origIndex = fFilteredContracts.index(index.row() - 1, index.column());
        }
        const QVariant result = fFilteredContracts.data(origIndex, role);
        return result;
    }

    void TokenModel::selectToken(int index) const
    {
        if ( index < 0 || index > fFilteredContracts.rowCount() ) {
            return;
        }

        if ( index == 0 ) {
            emit selectedTokenContract(-1);
            return;
        }

        index -= 1; // discount the ETH
        const QModelIndex modelIndex = fFilteredContracts.index(index, 0);
        const QModelIndex mappedIndex = fFilteredContracts.mapToSource(modelIndex);

        emit selectedTokenContract(mappedIndex.row());
    }

    void TokenModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
    {
        Q_UNUSED(topLeft);
        Q_UNUSED(bottomRight);
        Q_UNUSED(roles);

        beginResetModel();
        endResetModel();
    }

    void TokenModel::onRowsChanged(const QModelIndex &parent, int first, int last)
    {
        Q_UNUSED(parent);
        Q_UNUSED(first);
        Q_UNUSED(last);
        beginResetModel();
        endResetModel();
    }

}
