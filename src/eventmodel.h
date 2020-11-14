#ifndef EVENTMODEL_H
#define EVENTMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include "contractinfo.h"
#include "contractmodel.h"
#include "filtermodel.h"

namespace Etherwall {

    class EventModel : public QAbstractTableModel
    {
        Q_OBJECT
    public:
        EventModel(const ContractModel& contractModel, const FilterModel& filterModel);

        QHash<int, QByteArray> roleNames() const;
        int rowCount(const QModelIndex & parent __attribute__ ((unused))) const;
        Q_INVOKABLE virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role) const;
        Q_INVOKABLE const QString getName(int index) const;
        Q_INVOKABLE const QString getContract(int index) const;
        Q_INVOKABLE const QString getAddress(int index) const;
        Q_INVOKABLE const QString getData(int index) const;
        Q_INVOKABLE const QString getBlockNumber(int index) const;
        Q_INVOKABLE const QString getBlockHash(int index) const;
        Q_INVOKABLE const QString getTransactionHash(int index) const;
        Q_INVOKABLE const QString getTopics(int index) const;
        Q_INVOKABLE const QVariantList getArgModel(int index) const;
        Q_INVOKABLE const QString getParamValue(int index) const;
    public slots:
        void onNewEvent(const EventInfo& info, bool isNew);
        void onBeforeLoadLogs();
    signals:
        void receivedEvent(const QString& contract, const QString& signature);
    private:
        const ContractModel& fContractModel;
        EventList fList;
    };

}

#endif // EVENTMODEL_H
