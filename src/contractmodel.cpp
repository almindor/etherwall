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

    // contract model

    ContractModel::ContractModel(EtherIPC& ipc, AccountModel& accountModel) : QAbstractListModel(0),
        fList(), fIpc(ipc), fNetManager(), fBusy(false), fPendingContracts(), fAccountModel(accountModel)
    {
        connect(&accountModel, &AccountModel::accountsReady, this, &ContractModel::reload);
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
        if ( info.isERC20() ) {
            registerTokenWatch(info);
        }
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
        if ( fList.at(index).isERC20() ) { // remove watch for token
            fIpc.uninstallFilter(fList.at(index).address());
        }
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

    int ContractModel::getEventIndex(int index, const QJsonArray &topics) const
    {
        if ( index < 0 || index >= fList.size() ) {
            return -1;
        }

        if ( topics.size() < 1 || topics.at(0).toString().isEmpty() ) {
            return -1;
        }

        const QString methodID = Helpers::clearHexPrefix(topics.at(0).toString());
        return fList.at(index).eventIndexByMethodID(methodID);
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

    const QStringList ContractModel::getEvents(int index) const
    {
        if ( index < 0 || index >= fList.size() ) {
            return QStringList();
        }

        return fList.at(index).eventList();
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

    const QVariantList ContractModel::getEventArguments(int index, const QString& eventName, bool indexedOnly) const
    {
        if ( index < 0 || index >= fList.size() ) {
            return QVariantList();
        }

        try {
            int eventIndex = -1;
            return fList.at(index).event(eventName, eventIndex).getArgModel(indexedOnly);
        } catch ( QString err ) {
            EtherLog::logMsg(err, LS_Error);
            emit callError(err);
            return QVariantList();
        }
    }

    const QVariantList ContractModel::parseResponse(int contractIndex, const QString &data, const QVariantMap& userData) const
    {
        if ( !userData.contains("type") || userData.value("type").toString() != "functionCall" ) {
            EtherLog::logMsg("Invalid handler for call", LS_Error);
            return QVariantList(); // some other call result got here sadly
        }

        bool ok = false;
        int functionIndex = userData.value("functionIndex").toInt(&ok);

        if ( contractIndex < 0 || contractIndex >= fList.size() ) {
            emit callError("Invalid contract index in callIndex");
            return QVariantList();
        }

        if ( !ok || functionIndex < 0 || functionIndex >= fList.at(contractIndex).functionList().size() ) {
            emit callError("Invalid function index in callIndex");
            return QVariantList();
        }

        const ContractFunction func = fList.at(contractIndex).function(functionIndex);

        return func.parseResponse(data);
    }

    void ContractModel::encodeCall(int index, const QString& functionName, const QVariantList& params) {
        try {
            if ( index < 0 || index >= fList.size() ) {
                throw QString("Invalid contract index");
            }
            int funcIndex = -1;
            const ContractFunction func = fList.at(index).function(functionName, funcIndex);
            const QString encoded = "0x" + func.callData(params);
            QVariantMap userData;
            userData["functionIndex"] = QVariant(funcIndex);
            userData["type"] = "functionCall";
            emit callEncoded(encoded, func.isConstant(), index, userData);
        } catch ( QString err ) {
            emit callError(err);
        }
    }

    const QString ContractModel::encodeTopics(int index, const QString &eventName, const QVariantList &params)
    {
        try {
            if ( index < 0 || index >= fList.size() ) {
                throw QString("Invalid contract index");
            }
            int eventIndex = -1;
            const ContractEvent event = fList.at(index).event(eventName, eventIndex);
            const QJsonArray topics = event.encodeTopics(params);
            const QJsonDocument doc(topics);
            const QString encoded = QString::fromUtf8(doc.toJson());

            return encoded;
        } catch ( QString err ) {
            EtherLog::logMsg(err, LS_Error);
            return QString();
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

    bool ContractModel::callName(const QString& address, const QString &abiJson) const
    {
        QJsonParseError parseError;
        QJsonDocument abiDoc = QJsonDocument::fromJson(abiJson.toUtf8(), &parseError);
        if ( parseError.error != QJsonParseError::NoError ) {
            return false;
        }

        QJsonArray abi = abiDoc.array();
        foreach ( const QJsonValue& val, abi ) {
            const QJsonObject obj = val.toObject();
            if ( !obj.contains("outputs") || !obj.contains("name") || !obj.contains("constant") ) {
                continue;
            }

            const QString name = obj.value("name").toString("invalid");
            if ( name != "name" ) {
                continue;
            }

            const bool constant = obj.value("constant").toBool(false);
            if ( !constant ) {
                continue;
            }

            const QJsonArray outputs = obj.value("outputs").toArray();
            if ( outputs.size() != 1 ) {
                continue;
            }
            const QString outType = outputs.at(0).toObject().value("type").toString("invalid");
            if ( outType != "string" ) { // don't allow bytes, makes thing complex and most of those don't work right!
                continue;
            }

            QVariantMap userData;
            userData["type"] = QVariant("nameCall");
            Ethereum::Tx txName(QString(), address, QString(), 0, QString(), QString(), "0x06fdde03"); // hardcoded method id for name
            fIpc.call(txName, -1, userData);
            return true;
        }

        return false;
    }

    void ContractModel::reload() {
        QSettings settings;
        settings.beginGroup("contracts" + fIpc.getNetworkPostfix());
        const QStringList list = settings.allKeys();

        beginResetModel();
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
                if ( info.isERC20() ) {
                    registerTokenWatch(info);
                }
                index++;
            }
        }

        settings.endGroup();
        endResetModel();
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

    void ContractModel::onCallDone(const QString &result, int index, const QVariantMap& userData)
    {
        if ( !userData.contains("type") ) {
            EtherLog::logMsg("Missing type in call result user data", LS_Error);
            return;
        }

        const QString type = userData.value("type").toString();
        if ( type != "nameCall" && (index < 0 || index >= fList.size()) ) {
            EtherLog::logMsg("Invalid contract index from call result", LS_Error);
            return;
        }

        if ( type == "nameCall" ) { // no need for index or other checks here
            return onCallName(result);
        } else if ( type == "balanceCall" ) { // accounts handled elsewhere
            if ( !userData.contains("accountIndex") ) {
                return EtherLog::logMsg("Missing account index on token balance call", LS_Error);
            }
            bool ok = false;
            int accountIndex = userData.value("accountIndex").toInt(&ok);
            if ( !ok ) {
                return EtherLog::logMsg("Invalid account index on token balance call", LS_Error);
            }
            return onTokenBalance(result, index, accountIndex);
        }

        try {
            if ( type == "symbolCall" ) {
                fList[index].loadSymbolData(result);
            } else if ( type == "decimalsCall" ) {
                fList[index].loadDecimalsData(result);
            } else if ( type == "tokenNameCall" ) {
                fList[index].loadNameData(result);
            } else if ( type == "functionCall" ) {
                return; // safely ignore
            } else {
                EtherLog::logMsg("Unknown call response type: " + type, LS_Warning);
                return; // probably handled elsewhere
            }
        } catch (QString err) {
            EtherLog::logMsg("Error while loading token data: " + err, LS_Error);
            return;
        }

        const ContractInfo info = fList.at(index);
        QSettings settings;
        const QString lowerAddr = info.address().toLower();
        settings.beginGroup("contracts" + fIpc.getNetworkPostfix());
        settings.setValue(lowerAddr, info.toJsonString());
        settings.endGroup();

        emit dataChanged(QAbstractListModel::createIndex(index, 0), QAbstractListModel::createIndex(index, 0));
    }

    void ContractModel::onSelectedTokenContract(int index)
    {
        if ( index < 0 ) {
            fAccountModel.selectToken("ETH", QString());
        } else if ( index < fList.size() ) {
            const AccountList& accounts = fAccountModel.getAccounts();
            const ContractInfo& contract = fList.at(index);

            for ( int accountIndex = 0; accountIndex < accounts.size(); accountIndex++ ) {
                const QString accountAddress = accounts.at(accountIndex).hash();
                QVariantList params;
                params.append(accountAddress);

                int funcIndex = -1;
                const ContractFunction func = contract.function("balanceOf", funcIndex);
                const QString encoded = "0x" + func.callData(params);
                QVariantMap userData;
                userData["type"] = "balanceCall";
                userData["accountIndex"] = accountIndex;

                Ethereum::Tx txBalance(QString(), contract.address(), QString(), 0, QString(), QString(), encoded);
                fIpc.call(txBalance, index, userData);
            }

            fAccountModel.selectToken(contract.token(), contract.address());
        } else {
            EtherLog::logMsg("Token selection out of contract bounds", LS_Error);
        }
    }

    void ContractModel::loadERC20Data(const ContractInfo &contract, int index) const
    {
        int i; // unused

        // symbol
        QVariantMap symbolData;
        symbolData["type"] = "symbolCall";
        Ethereum::Tx txSymbol(QString(), contract.address(), QString(), 0, QString(), QString(), contract.function("symbol", i).getMethodID());
        fIpc.call(txSymbol, index, symbolData);
        // decimals
        QVariantMap decimalsData;
        decimalsData["type"] = "decimalsCall";
        Ethereum::Tx txDecimals(QString(), contract.address(), QString(), 0, QString(), QString(), contract.function("decimals", i).getMethodID());
        fIpc.call(txDecimals, index, decimalsData);
        // name if required
        if ( contract.name().isEmpty() ) {
            QVariantMap nameData;
            nameData["type"] = "tokenNameCall";
            Ethereum::Tx txName(QString(), contract.address(), QString(), 0, QString(), QString(), contract.function("name", i).getMethodID());
            fIpc.call(txName, index, nameData);
        }
    }

    void ContractModel::onCallName(const QString &result) const
    {
        ContractArg arg("name", "string");
        QString prepared = result;
        if ( prepared.startsWith("0x") ) {
            prepared.remove(0, 2); // remove 0x
        }

        int dynamicOffset = arg.decodeInt(prepared.left(64), false).toUlong() * 2;
        const QString raw = prepared.mid(dynamicOffset); // dynamic types cut off themselves
        const QVariant decoded = arg.decode(raw);

        emit callNameDone(decoded.toString());
    }

    void ContractModel::onTokenBalance(const QString &result, int contractIndex, int accountIndex) const
    {
        if ( contractIndex < 0 || contractIndex >= fList.size() ) {
            return EtherLog::logMsg("Invalid contract index on token balance call", LS_Error);
        }

        if ( accountIndex < 0 ) { // upper bound check in accountModel
            return EtherLog::logMsg("Invalid account index on token balance call", LS_Error);
        }

        int i;
        QVariantList parsedSet = fList.at(contractIndex).function("balanceOf", i).parseResponse(result);
        if ( parsedSet.size() != 1 ) {
            return EtherLog::logMsg("Invalid response size for token balanceOf call", LS_Error);
        }
        const QString balanceBase = parsedSet.at(0).toMap().value("value").toString();
        // we need to get decimals for contract/token and then get the "full" units
        const QString balanceFull = Helpers::baseStrToFullStr(balanceBase, fList.at(contractIndex).decimals());

        emit tokenBalanceDone(accountIndex, fList.at(contractIndex).address(), balanceFull);
    }

    void ContractModel::registerTokenWatch(const ContractInfo &contract)
    {
        QVariantList params;
        params.append(QVariant());
        const QVariantList addresses = fAccountModel.getAccountAddresses();
        params.insert(params.size(), addresses);
        int eventIndex; // out

        try {
            const ContractEvent event = contract.event("Transfer", eventIndex);
            const QJsonArray topics = event.encodeTopics(params);

            fIpc.newEventFilter(contract.address(), topics);
            qDebug() << "added new filter with topics: " << topics << "\n";
        } catch ( QString error ) {
            return EtherLog::logMsg(error, LS_Error);
        }
    }

}
