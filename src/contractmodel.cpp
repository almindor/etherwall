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
#include "helpers.h"
#include <QSettings>
#include <QJsonDocument>
#include <QDebug>

namespace Etherwall {

    PendingContract::PendingContract() {
        fName = "invalid";
        fAbi = "invalid";
    }

    PendingContract::PendingContract(const QString& name, const QString& abi)
        : fName(name), fAbi(abi)
    {
    }

    ContractModel::ContractModel(EtherIPC& ipc) : QAbstractListModel(0), fList(), fIpc(ipc), fNetManager(), fBusy(false), fPendingContracts()
    {
        connect(&ipc, &EtherIPC::connectToServerDone, this, &ContractModel::reload);
        connect(&ipc, &EtherIPC::newEvent, this, &ContractModel::onNewEvent);
        connect(&ipc, &EtherIPC::callDone, this, &ContractModel::onCallDone);
        connect(&fNetManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(httpRequestDone(QNetworkReply*)));
    }

    QHash<int, QByteArray> ContractModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[ContractNameRole] = "name";
        roles[AddressRole] = "address";
        roles[TokenRole] = "token";
        roles[DecimalsRole] = "decimals";
        roles[ABIRole] = "abi";

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
        const QString lowerAddr = info.value(AddressRole).toString().toLower();
        settings.beginGroup("contracts" + fIpc.getNetworkPostfix());
        if ( settings.contains(info.value(AddressRole).toString()) ) { // we didn't lowercase before
            settings.remove(info.value(AddressRole).toString());
        }
        settings.setValue(lowerAddr, info.toJsonString());
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

        if ( info.needsERC20Init() ) {
            loadERC20Data(info, fList.size() - 1);
        }

        return true;
    }

    bool ContractModel::addPendingContract(const QString& name, const QString& abi, const QString& hash) {
        fPendingContracts[hash] = PendingContract(name, abi);
        return true;
    }

    const QString ContractModel::contractDeployed(const QJsonObject& receipt) {
        const QString hash = receipt.value("transactionHash").toString("invalid");
        if ( hash == "invalid" ) {
            EtherLog::logMsg("Contract deployment receipt missing transaction hash", LS_Error);
            return QString();
        }

        if ( !fPendingContracts.contains(hash) ) {
            EtherLog::logMsg("Contract deployment transaction hash mismatch", LS_Error);
            return QString();
        }

        const QString address = receipt.value("contractAddress").toString("invalid");
        if ( address == "invalid" || address.isEmpty() ) {
            EtherLog::logMsg("Contract address invalid in tx receipt", LS_Error);
            return QString();
        }

        const PendingContract pc = fPendingContracts.value(hash);
        addContract(pc.fName, address, pc.fAbi);
        fPendingContracts.remove(hash);

        return pc.fName;
    }

    bool ContractModel::deleteContract(int index) {
        if ( index < 0 || index >= fList.size() ) {
            return false;
        }

        QSettings settings;
        settings.beginGroup("contracts" + fIpc.getNetworkPostfix());
        settings.remove(fList.at(index).address()); // we didn't lowercase before
        settings.remove(fList.at(index).address().toLower());
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

    int ContractModel::getIndex(const QString name) const
    {
        for ( int i = 0; i < fList.size(); i++ ) {
            if ( fList.at(i).name() == name ) {
                return i;
            }
        }

        return -1;
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

        return fList.at(index).functionList();
    }

    const QString ContractModel::getMethodID(int index, const QString& functionName) const {
        if ( index < 0 || index >= fList.size() ) {
            return QString();
        }

        try {
            int funcIndex = -1;
            return fList.at(index).function(functionName, funcIndex).getMethodID();
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
            int funcIndex = -1;
            return fList.at(index).function(functionName, funcIndex).getArgModel();
        } catch ( QString err ) {
            EtherLog::logMsg(err, LS_Error);
            emit callError(err);
            return QVariantList();
        }
    }

    const QVariantList ContractModel::parseResponse(int callIndex, const QString &data) const
    {
        int contractIndex = callIndex / 100000;
        int functionIndex = callIndex - (contractIndex * 100000);

        if ( contractIndex < 0 || contractIndex >= fList.size() ) {
            emit callError("Invalid contract index in callIndex");
            return QVariantList();
        }

        if ( functionIndex < 0 || functionIndex >= fList.at(contractIndex).functionList().size() ) {
            emit callError("Invalid function index in callIndex");
            return QVariantList();
        }

        const ContractFunction func = fList.at(contractIndex).function(functionIndex);

        return func.parseResponse(data);
    }

    void ContractModel::encodeCall(int index, const QString& functionName, const QVariantList& params) {
        try {
            int funcIndex = -1;
            const ContractFunction func = fList.at(index).function(functionName, funcIndex);
            const int callIndex = index * 100000 + funcIndex; // we need this to "parse the result" properly so we can match it to the calling contract/function when it comes back
            const QString encoded = "0x" + func.callData(params);
            emit callEncoded(encoded, func.isConstant(), callIndex);
        } catch ( QString err ) {
            emit callError(err);
        }
    }

    void ContractModel::requestAbi(const QString& address) {
        // get contract ABI
        QNetworkRequest request(QUrl("https://data.etherwall.com/api/contracts"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QJsonObject objectJson;
        objectJson["address"] = address;
        if ( fIpc.getTestnet() ) {
            objectJson["testnet"] = true;
        }
        const QByteArray data = QJsonDocument(objectJson).toJson();

        EtherLog::logMsg("HTTP Post request: " + data, LS_Debug);

        fNetManager.post(request, data);
        fBusy = true;
        emit busyChanged(true);
    }

    void ContractModel::reload() {
        QSettings settings;
        settings.beginGroup("contracts" + fIpc.getNetworkPostfix());
        const QStringList list = settings.allKeys();

        int index = 0;
        foreach ( const QString addr, list ) {
            QJsonParseError parseError;
            const QString val = settings.value(addr).toString();
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8(), &parseError);

            if ( parseError.error != QJsonParseError::NoError ) {
                EtherLog::logMsg("Error parsing stored contract: " + parseError.errorString(), LS_Error);
            } else {
                const ContractInfo info(jsonDoc.object());
                if ( info.needsERC20Init() ) {
                    loadERC20Data(info, index);
                }
                fList.append(info);
                index++;
            }
        }

        settings.endGroup();
    }

    void ContractModel::onNewEvent(const QJsonObject& event, bool isNew) {
        EventInfo info(event);

        // find the right contract and process/fill the params
        foreach ( const ContractInfo ci, fList ) {
            if ( ci.address() == info.address() ) {
                ci.processEvent(info);
            }
        }

        emit newEvent(info, isNew);
    }

    void ContractModel::httpRequestDone(QNetworkReply *reply) {
        QJsonObject resObj = Helpers::parseHTTPReply(reply);
        const bool success = resObj.value("success").toBool();

        fBusy = false;
        emit busyChanged(false);

        if ( !success ) {
            return; // probably just unknown ABI/contract
        }

        const QJsonValue rv = resObj.value("abi");
        const QJsonArray arr = rv.toArray();
        const QJsonDocument doc(arr);
        const QString result(doc.toJson());

        emit abiResult(result);
    }

    void ContractModel::onCallDone(const QString &result, int index)
    {
        int actualIndex;
        if ( index <= -10000 ) {
            actualIndex = (index * -1) - 10000;
        } else if ( index <= 1000 ) {
            actualIndex = (index * -1) - 1000;
        } else {
            return; // not internal
        }

        if ( actualIndex < 0 || actualIndex >= fList.size() ) {
            EtherLog::logMsg("Invalid call index for token contract symbol load: " + QString::number(index), LS_Error);
            return;
        }

        try {
            if ( index <= -10000 ) {
                fList[actualIndex].loadDecimalsData(result);
            } else if ( index <= -1000 ) {
                fList[actualIndex].loadSymbolData(result);
            } else {
                EtherLog::logMsg("Invalid index type", LS_Error); // redundant
            }
        } catch (QString err) {
            EtherLog::logMsg("Error while loading token data: " + err, LS_Error);
            return;
        }

        const ContractInfo info = fList.at(actualIndex);
        QSettings settings;
        const QString lowerAddr = info.address().toLower();
        settings.beginGroup("contracts" + fIpc.getNetworkPostfix());
        settings.setValue(lowerAddr, info.toJsonString());
        settings.endGroup();

        emit dataChanged(QAbstractListModel::createIndex(actualIndex, 0), QAbstractListModel::createIndex(actualIndex, 0));
    }

    void ContractModel::loadERC20Data(const ContractInfo &contract, int index) const
    {
        // symbol
        int symbolIndex = -1000 - index;
        Ethereum::Tx txSymbol(QString(), contract.address(), QString(), 0, QString(), QString(), contract.symbolCallData());
        fIpc.call(txSymbol, symbolIndex);
        // decimals
        int decimalsIndex = -10000 - index;
        Ethereum::Tx txDecimals(QString(), contract.address(), QString(), 0, QString(), QString(), contract.decimalsCallData());
        fIpc.call(txDecimals, decimalsIndex);
    }

}
