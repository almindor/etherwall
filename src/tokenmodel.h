#ifndef TOKENMODEL_H
#define TOKENMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QAbstractListModel>

namespace Etherwall {

    class TokenModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        TokenModel(QAbstractItemModel* source);

        QHash<int, QByteArray> roleNames() const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

        Q_INVOKABLE void selectToken(int index) const;
    signals:
        void selectedTokenContract(int index) const;
    private slots:
        void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
        void onRowsChanged(const QModelIndex &parent, int first, int last);
    private:
        QSortFilterProxyModel fFilteredContracts;
    };

}

#endif // TOKENMODEL_H
