#include "eventmodel.h"

namespace Etherwall {

    EventModel::EventModel(const ContractModel& contractModel) : QAbstractListModel(0), fContractModel(contractModel), fList()
    {
        connect(&contractModel, &ContractModel::newEvent, this, &EventModel::newEvent);
    }

    QHash<int, QByteArray> EventModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[EventNameRole] = "name";
        roles[EventAddressRole] = "address";
        roles[EventBlockHashRole] = "blockhash";
        roles[EventBlockNumberRole] = "blocknumber";
        roles[EventTransactionHashRole] = "transactionhash";
        roles[EventArgumentsRole] = "arguments";
        roles[EventParamsRole] = "params";

        return roles;
    }

    int EventModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const {
        return fList.size();
    }

    QVariant EventModel::data(const QModelIndex & index, int role) const {
        return fList.at(index.row()).value(role);
    }

    void EventModel::newEvent(const EventInfo& info) {
        beginInsertRows(QModelIndex(), fList.length(), fList.length());
        fList.append(info);
        endInsertRows();
    }

}
