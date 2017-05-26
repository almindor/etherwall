#include "filtermodel.h"
#include <QSettings>

namespace Etherwall {

    FilterModel::FilterModel(EtherIPC& ipc) : QAbstractListModel(0), fIpc(ipc), fList()
    {
        connect(&ipc, &EtherIPC::connectToServerDone, this, &FilterModel::reload);
    }

    QHash<int, QByteArray> FilterModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[FilterNameRole] = "name";
        roles[FilterAddressRole] = "address";
        roles[FilterContractRole] = "contract";
        roles[FilterTopicsRole] = "topics";
        roles[FilterActiveRole] = "active";

        return roles;
    }

    int FilterModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const {
        return fList.size();
    }

    QVariant FilterModel::data(const QModelIndex & index, int role) const {
        return fList.at(index.row()).value(role);
    }

    const QString FilterModel::getName(int index) const {
        if ( index < 0 || index >= fList.length() ) {
            return QString();
        }

        return fList.at(index).value(FilterNameRole).toString();
    }

    const QString FilterModel::getContract(int index) const {
        if ( index < 0 || index >= fList.length() ) {
            return QString();
        }

        return fList.at(index).value(FilterContractRole).toString();
    }

    const QString FilterModel::getTopics(int index) const {
        if ( index < 0 || index >= fList.length() ) {
            return QString();
        }

        return fList.at(index).value(FilterTopicsRole).toStringList().join(',');
    }

    bool FilterModel::getActive(int index) const {
        if ( index < 0 || index >= fList.length() ) {
            return false;
        }

        return fList.at(index).value(FilterActiveRole).toBool();
    }

    void FilterModel::addFilter(const QString& name, const QString& address, const QString& contract, const QString& topics, bool active) {
        const FilterInfo info(name, address, contract, topics.split(","), active);
        QSettings settings;
        settings.beginGroup("filters" + fIpc.getNetworkPostfix());
        settings.setValue(info.getHandle(), info.toJsonString());
        settings.endGroup();

        // check if it's an update
        for ( int i = 0; i < fList.length(); i++ ) {
            const FilterInfo fi = fList.at(i);
            if ( fi.getHandle() == info.getHandle() ) {
                fList[i] = info;

                update(i);
                return;
            }
        }

        beginInsertRows(QModelIndex(), fList.length(), fList.length());
        fList.append(info);
        endInsertRows();

        registerFilters();
        loadLogs();
    }

    void FilterModel::setFilterActive(int index, bool active) {
        if ( index < 0 || index >= fList.length() ) {
            return;
        }

        fList[index].setActive(active);
        const FilterInfo info = fList.at(index);

        QSettings settings;
        settings.beginGroup("filters" + fIpc.getNetworkPostfix());
        settings.setValue(info.getHandle(), info.toJsonString());
        settings.endGroup();

        update(index);
    }

    void FilterModel::deleteFilter(int index) {
        if ( index < 0 || index >= fList.length() ) {
            return;
        }

        const FilterInfo info = fList.at(index);
        QSettings settings;
        settings.beginGroup("filters" + fIpc.getNetworkPostfix());
        settings.remove(info.getHandle());
        settings.endGroup();

        beginRemoveRows(QModelIndex(), index, index);
        fList.removeAt(index);
        endRemoveRows();

        registerFilters();
    }

    void FilterModel::update(int index) {
        const QModelIndex& leftIndex = QAbstractListModel::createIndex(index, 0);
        const QModelIndex& rightIndex = QAbstractListModel::createIndex(index, 3);
        QVector<int> roles(4);
        roles[0] = FilterNameRole;
        roles[1] = FilterContractRole;
        roles[2] = FilterTopicsRole;
        roles[3] = FilterActiveRole;
        emit dataChanged(leftIndex, rightIndex, roles);

        registerFilters();
        if ( fList.at(index).value(FilterActiveRole).toBool() ) {
            loadLogs();
        }
    }

    void FilterModel::registerFilters() const {
        QStringList addresses;
        QStringList topics;

        foreach ( const FilterInfo info, fList ) {
            if ( !info.value(FilterActiveRole).toBool() ) {
                continue;
            }

            addresses.append(info.value(FilterAddressRole).toString());
            const QStringList infoTopics = info.value(FilterTopicsRole).toStringList();
            if ( infoTopics.length() > 0 ) {
                topics += infoTopics;
            }
        }

        fIpc.registerEventFilters(addresses, topics);
    }

    void FilterModel::loadLogs() const {
        const QSettings settings;
        quint64 day = settings.value("geth/logsize", 7200).toLongLong();
        quint64 fromBlock = fIpc.blockNumber() > day ? fIpc.blockNumber() - day : 1;

        emit beforeLoadLogs();
        foreach ( const FilterInfo info, fList ) {
            if ( !info.value(FilterActiveRole).toBool() ) {
                continue;
            }

            QStringList addresses;
            addresses.append(info.value(FilterAddressRole).toString());
            const QStringList infoTopics = info.value(FilterTopicsRole).toStringList();
            fIpc.loadLogs(addresses, infoTopics, fromBlock);
        }
    }

    void FilterModel::reload() {
        QSettings settings;
        settings.beginGroup("filters" + fIpc.getNetworkPostfix());

        const QStringList list = settings.allKeys();

        foreach ( const QString addr, list ) {
            QJsonParseError parseError;
            const QString val = settings.value(addr).toString();
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8(), &parseError);

            if ( parseError.error != QJsonParseError::NoError ) {
                EtherLog::logMsg("Error parsing stored filter: " + parseError.errorString(), LS_Error);
            } else {
                const FilterInfo info(jsonDoc.object());
                fList.append(info);
            }
        }

        settings.endGroup();

        registerFilters();
        loadLogs();
    }

}
