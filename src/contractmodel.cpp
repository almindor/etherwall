/*
    This file is part of etherwall.
    etherwall is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    etherwall is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with etherwall. If not, see <http://www.gnu.org/licenses/>.
*/
/** @file contractmodel.cpp
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * Contract model body
 */

#include "contractmodel.h"
#include "etherlog.h"
#include <QSettings>
#include <QJsonDocument>
#include <QDebug>

namespace Etherwall {

    ContractModel::ContractModel(EtherIPC& ipc) : QAbstractListModel(0), fList(), fIpc(ipc)
    {
        connect(&ipc, &EtherIPC::connectToServerDone, this, &ContractModel::reload);
    }

    QHash<int, QByteArray> ContractModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[ContractRoles::ContractNameRole] = "name";
        roles[ContractRoles::AddressRole] = "address";
        roles[ContractRoles::ABIRole] = "abi";

        return roles;
    }

    int ContractModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const {
        return fList.size();
    }

    QVariant ContractModel::data(const QModelIndex & index, int role) const {
        return fList.at(index.row()).value(role);
    }

    bool ContractModel::addContract(const QString& name, const QString& address, const QString& abi) {
        QJsonParseError parseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(abi.toUtf8(), &parseError);

        // shouldn't happen as we check from QML
        if ( parseError.error != QJsonParseError::NoError ) {
            EtherLog::logMsg("Error parsing new contract: " + parseError.errorString(), LS_Error);
            return false;
        }

        if ( !jsonDoc.isArray() ) {
            EtherLog::logMsg("Contract ABI not an array", LS_Error);
            return false;
        }

        const ContractInfo info(name, address, jsonDoc.array());

        QSettings settings;
        settings.beginGroup("contracts" + fIpc.getNetworkPostfix());
        settings.setValue(info.value(AddressRole).toString(), info.toJsonString());
        settings.endGroup();

        int at = 0;
        foreach ( const ContractInfo li, fList ) {
            if ( li.address() == info.address() ) {
                fList[at] = info;

                QVector<int> roles(3);
                roles[0] = ContractNameRole;
                roles[1] = AddressRole;
                roles[2] = ABIRole;
                const QModelIndex& leftIndex = QAbstractListModel::createIndex(at, 0);
                const QModelIndex& rightIndex = QAbstractListModel::createIndex(at, 0);

                emit dataChanged(leftIndex, rightIndex, roles);
                return true;
            }
            at++;
        }

        beginInsertRows(QModelIndex(), fList.size(), fList.size());
        fList.append(info);
        endInsertRows();

        return true;
    }

    bool ContractModel::deleteContract(int index) {
        if ( index < 0 || index >= fList.size() ) {
            return false;
        }

        QSettings settings;
        settings.beginGroup("contracts" + fIpc.getNetworkPostfix());
        settings.remove(fList.at(index).address());
        settings.endGroup();

        beginRemoveRows(QModelIndex(), index, index);
        fList.removeAt(index);
        endRemoveRows();

        return true;
    }

    const QString ContractModel::getName(int index) const {
        if ( index < 0 || index >= fList.size() ) {
            return QString();
        }

        return fList.at(index).name();
    }

    const QString ContractModel::getAddress(int index) const {
        if ( index < 0 || index >= fList.size() ) {
            return QString();
        }

        return fList.at(index).address();
    }

    const QString ContractModel::getABI(int index) const {
        if ( index < 0 || index >= fList.size() ) {
            return QString();
        }

        return fList.at(index).abi();
    }

    const QStringList ContractModel::getFunctions(int index) const {
        if ( index < 0 || index >= fList.size() ) {
            return QStringList();
        }

        QStringList list(fList.at(index).functionList());
        list.insert(0, "Select function");
        return list;
    }

    const QString ContractModel::getMethodID(int index, const QString& functionName) const {
        if ( index < 0 || index >= fList.size() ) {
            return QString();
        }

        try {
            return fList.at(index).function(functionName).getMethodID();
        } catch ( QString err ) {
            EtherLog::logMsg(err, LS_Error);
            return QString();
        }
    }

    const QVariantList ContractModel::getArguments(int index, const QString& functionName) const {
        if ( index < 0 || index >= fList.size() ) {
            return QVariantList();
        }

        try {
            return fList.at(index).function(functionName).getArgModel();
        } catch ( QString err ) {
            EtherLog::logMsg(err, LS_Error);
            emit callError(err);
            return QVariantList();
        }
    }

    void ContractModel::encodeCall(int index, const QString& functionName, const QVariantList& params) {
        try {
            const QString encoded = "0x" + fList.at(index).function(functionName).callData(params);
            emit callEncoded(encoded);
        } catch ( QString err ) {
            EtherLog::logMsg(err, LS_Error);
            emit callError(err);
        }
    }

    void ContractModel::reload() {
        QSettings settings;
        settings.beginGroup("contracts" + fIpc.getNetworkPostfix());
        const QStringList list = settings.allKeys();

        foreach ( const QString addr, list ) {
            QJsonParseError parseError;
            const QString val = settings.value(addr).toString();
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8(), &parseError);

            if ( parseError.error != QJsonParseError::NoError ) {
                EtherLog::logMsg("Error parsing stored contract: " + parseError.errorString(), LS_Error);
            } else {
                const ContractInfo info(jsonDoc.object());
                fList.append(info);
            }
        }

        settings.endGroup();
    }

}
