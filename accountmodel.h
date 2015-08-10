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
    private:
        AccountList fAccountList;
        EtherIPC fIpc;

        void refresh();
    signals:

    public slots:

    };

}

#endif // ACCOUNTMODEL_H
