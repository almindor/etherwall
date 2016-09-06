#ifndef FILTERMODEL_H
#define FILTERMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "contractinfo.h"
#include "etheripc.h"

namespace Etherwall {

    class FilterModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        FilterModel(EtherIPC& ipc);

        QHash<int, QByteArray> roleNames() const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

        Q_INVOKABLE const QString getName(int index) const;
        Q_INVOKABLE const QString getContract(int index) const;
        Q_INVOKABLE const QString getTopics(int index) const;
        Q_INVOKABLE bool getActive(int index) const;

        Q_INVOKABLE void addFilter(const QString& name, const QString& address, const QString& contract, const QString& topics, bool active);
        Q_INVOKABLE void setFilterActive(int index, bool active);
        Q_INVOKABLE void deleteFilter(int index);
        Q_INVOKABLE void loadLogs() const;
    public slots:
        void reload();
    signals:
        void beforeLoadLogs() const;
    private:
        void update(int index);
        void registerFilters() const;
        EtherIPC& fIpc;
        EventFilters fList;
    };

}

#endif // FILTERMODEL_H
