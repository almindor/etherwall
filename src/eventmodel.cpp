#include "eventmodel.h"

namespace Etherwall {

    EventModel::EventModel(const ContractModel& contractModel, const FilterModel& filterModel) :
        QAbstractTableModel(0), fContractModel(contractModel), fList()
    {
        connect(&contractModel, &ContractModel::newEvent, this, &EventModel::onNewEvent);
        connect(&filterModel, &FilterModel::beforeLoadLogs, this, &EventModel::onBeforeLoadLogs);
    }

    QHash<int, QByteArray> EventModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[Qt::DisplayRole] = "display";
        roles[EventNameRole] = "name";
        roles[EventContractRole] = "contract";
        roles[EventAddressRole] = "address";
        roles[EventDataRole] = "data";
        roles[EventTopicsRole] = "topics";
        roles[EventBlockHashRole] = "blockhash";
        roles[EventBlockNumberRole] = "blocknumber";
        roles[EventTransactionHashRole] = "transactionhash";

        return roles;
    }

    int EventModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const {
        return fList.size();
    }

    int EventModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);

        return 3;
    }

    QVariant EventModel::data(const QModelIndex & index, int role) const {
        int row = index.row();

        if ( role == Qt::DisplayRole ) {
            switch ( index.column() ) {
                case 0: return fList.at(row).value(EventNameRole);
                case 1: return fList.at(row).contract();
                case 2: return fList.at(row).blockNumber();
            }

            return "?";
        }

        return fList.at(row).value(role);
    }

    const QString EventModel::getName(int index) const {
        if ( index < 0 || index >= fList.length() ) {
            return QString();
        }

        return fList.at(index).value(EventNameRole).toString();
    }

    const QString EventModel::getContract(int index) const {
        if ( index < 0 || index >= fList.length() ) {
            return QString();
        }

        return fList.at(index).value(EventContractRole).toString();
    }

    const QString EventModel::getAddress(int index) const {
        if ( index < 0 || index >= fList.length() ) {
            return QString();
        }

        return fList.at(index).address();
    }

    const QString EventModel::getData(int index) const {
        if ( index < 0 || index >= fList.length() ) {
            return QString();
        }

        return fList.at(index).value(EventDataRole).toString();
    }

    const QString EventModel::getBlockNumber(int index) const {
        if ( index < 0 || index >= fList.length() ) {
            return QString();
        }

        return fList.at(index).value(EventBlockNumberRole).toString();
    }

    const QString EventModel::getBlockHash(int index) const {
        if ( index < 0 || index >= fList.length() ) {
            return QString();
        }

        return fList.at(index).value(EventBlockHashRole).toString();
    }

    const QString EventModel::getTransactionHash(int index) const {
        if ( index < 0 || index >= fList.length() ) {
            return QString();
        }

        return fList.at(index).transactionHash();
    }

    const QString EventModel::getTopics(int index) const {
        if ( index < 0 || index >= fList.length() ) {
            return QString();
        }

        return fList.at(index).value(EventTopicsRole).toString();
    }

    const QVariantList EventModel::getArgModel(int index) const {
        if ( index < 0 || index >= fList.length() ) {
            return QVariantList();
        }

        const ContractArgs args = fList.at(index).getArguments();
        const QVariantList params = fList.at(index).getParams();
        QVariantList result;
        QVariantMap map;

        int i = 0;
        foreach ( const ContractArg arg, args ) {
            map["name"] = arg.name();
            map["type"] = arg.type();
            QString strVal = fList.at(index).paramToStr(params.at(i++));
            map["value"] = strVal;
            result.append(map);
        }

        return result;
    }

    const QString EventModel::getParamValue(int index) const {
        if ( index < 0 || index >= fList.length() ) {
            return QString();
        }

        const QVariant value = fList.at(index).getParams().at(index);
        QString strVal = fList.at(index).paramToStr(value);

        return strVal;
    }

    void EventModel::onNewEvent(const EventInfo& info, bool isNew) {
        // sort by block number descending
        int index = 0;
        while ( index < fList.length() && fList.at(index).blockNumber() > info.blockNumber() ) {
            index++;
        }

        beginInsertRows(QModelIndex(), index, index);
        fList.insert(index, info);
        endInsertRows();

        if ( isNew ) {
            emit receivedEvent(info.contract(), info.signature());
        }
    }

    void EventModel::onBeforeLoadLogs() {
        beginResetModel();
        fList.clear();
        endResetModel();
    }

}
