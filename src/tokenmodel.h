#ifndef TOKENMODEL_H
#define TOKENMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QAbstractListModel>
#include "contractmodel.h"

namespace Etherwall {

    class TokenModel : public QAbstractListModel
    {
        Q_OBJECT
        Q_PROPERTY(int outerIndex READ getOuterIndex NOTIFY outerIndexChanged)
    public:
        TokenModel(ContractModel* source);

        QHash<int, QByteArray> roleNames() const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        int getOuterIndex() const;

        Q_INVOKABLE void selectToken(int index);
        Q_INVOKABLE const QString getTokenAddress(int index) const;
        Q_INVOKABLE int getTokenDecimals(int index) const;
        Q_INVOKABLE const QString getTokenTransferData(int index, const QString& toAddress, const QString& value) const;
    signals:
        void selectedTokenContract(int index, bool forAccounts) const;
        void outerIndexChanged(int index) const;
    private slots:
        void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
        void onRowsChanged(const QModelIndex &parent, int first, int last);
    private:
        QSortFilterProxyModel fFilteredContracts;
        ContractModel& fContractModel;
        int fOuterIndex;

        int mapIndex(int index) const;
    };

}

#endif // TOKENMODEL_H
