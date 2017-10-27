#include "tokenmodel.h"
#include "contractmodel.h"
#include <QDebug>

namespace Etherwall {

    TokenModel::TokenModel(ContractModel* source) :
        QAbstractListModel(0), fContractModel(*source)
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
        roles[AddressRole] = "address";
        roles[TokenRole] = "token";
        roles[DecimalsRole] = "decimals";

        return roles;
    }

    int TokenModel::rowCount(const QModelIndex &parent) const
    {
        return fFilteredContracts.rowCount(parent) + 1;
    }

    QVariant TokenModel::data(const QModelIndex &index, int role) const
    {        
        if ( role != TokenRole && role != DecimalsRole && role != AddressRole ) {
            return QVariant("Invalid role");
        }

        if ( index.row() == 0 ) {
            if ( role == TokenRole ) {
                return QVariant("ETH");
            } else if ( role == DecimalsRole ) {
                return QVariant(18);
            } else if ( role == AddressRole ) {
                return QVariant("0x0000000000000000000000000000000000000000");
            }
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
            emit selectedTokenContract(-1, true);
            return;
        }

        emit selectedTokenContract(mapIndex(index), true);
    }

    const QString TokenModel::getTokenAddress(int index) const
    {
        if ( index < 0 || index > fFilteredContracts.rowCount() ) {
            return QString("invalid");
        }

        if ( index == 0 ) {
            return QString("0x000000000000000000000000000000000000000");
        }

        int mapped = mapIndex(index);

        return fContractModel.getAddress(mapped);
    }

    int TokenModel::getTokenDecimals(int index) const
    {
        if ( index < 0 || index > fFilteredContracts.rowCount() ) {
            return -1;
        }

        if ( index == 0 ) {
            return 18;
        }

        int mapped = mapIndex(index);
        return fContractModel.getDecimals(mapped);
    }

    const QString TokenModel::getTokenTransferData(int index, const QString &toAddress, const QString& value) const
    {
        if ( index < 0 || index > fFilteredContracts.rowCount() ) {
            return QString();
        }

        if ( index == 0 ) {
            return QString();
        }

        int mapped = mapIndex(index);
        return fContractModel.encodeTransfer(mapped, toAddress, value);
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

    int TokenModel::mapIndex(int index) const
    {
        index -= 1; // discount the ETH
        const QModelIndex modelIndex = fFilteredContracts.index(index, 0);
        const QModelIndex mappedIndex = fFilteredContracts.mapToSource(modelIndex);

        return mappedIndex.row();
    }

}
