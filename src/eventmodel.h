#ifndef EVENTMODEL_H
#define EVENTMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "contractinfo.h"
#include "contractmodel.h"

namespace Etherwall {

    class EventModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        EventModel(const ContractModel& contractModel);

        QHash<int, QByteArray> roleNames() const;
        int rowCount(const QModelIndex & parent __attribute__ ((unused))) const;
        QVariant data(const QModelIndex & index, int role) const;
    public slots:
        void newEvent(const EventInfo& info);
    private:
        const ContractModel& fContractModel;
        EventList fList;
    };

}

#endif // EVENTMODEL_H
