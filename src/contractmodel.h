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
 * Contract model header
 */

#ifndef CONTRACTMODEL_H
#define CONTRACTMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QNetworkAccessManager>
#include <QVariantList>
#include <QVariantMap>
#include "contractinfo.h"
#include "nodeipc.h"
#include "accountmodel.h"

namespace Etherwall {

    class PendingContract {
    public:
        PendingContract();
        PendingContract(const QString& name, const QString& abi);
        QString fName;
        QString fAbi;
    };

    typedef QMap<QString, PendingContract> PendingContracts;

    class ContractModel : public QAbstractListModel
    {
        Q_OBJECT
        Q_PROPERTY(bool busy MEMBER fBusy NOTIFY busyChanged)
    public:
        ContractModel(NodeIPC& ipc, AccountModel& accountModel);

        QHash<int, QByteArray> roleNames() const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        Q_INVOKABLE bool addContract(const QString& name, const QString& address, const QString& abi);
        Q_INVOKABLE bool addPendingContract(const QString& name, const QString& abi, const QString& hash);
        Q_INVOKABLE const QString contractDeployed(const QJsonObject& receipt);
        Q_INVOKABLE bool deleteContract(int index);
        Q_INVOKABLE const QString getName(int index) const;
        Q_INVOKABLE int getIndex(const QString name) const;
        Q_INVOKABLE int getEventIndex(int index, const QJsonArray& topics) const;
        Q_INVOKABLE const QString getAddress(int index) const;
        Q_INVOKABLE int getDecimals(int index) const;
        Q_INVOKABLE const QString getABI(int index) const;
        Q_INVOKABLE const QStringList getFunctions(int index) const;
        Q_INVOKABLE const QStringList getEvents(int index) const;
        Q_INVOKABLE const QString getMethodID(int index, const QString& functionName) const;
        Q_INVOKABLE const QVariantList getArguments(int index, const QString& functionName) const;
        Q_INVOKABLE const QVariantList getEventArguments(int index, const QString& eventName, bool indexedOnly) const;
        Q_INVOKABLE const QVariantList parseResponse(int contractIndex, const QString& data, const QVariantMap& userData) const;
        Q_INVOKABLE void encodeCall(int index, const QString& functionName, const QVariantList& params);
        Q_INVOKABLE const QString encodeTransfer(int index, const QString& toAddress, const QString& value);
        Q_INVOKABLE const QString encodeTopics(int index, const QString& eventName, const QVariantList& params);
        Q_INVOKABLE void requestAbi(const QString& address);
        Q_INVOKABLE bool callName(const QString& address, const QString& jsonAbi) const;
    signals:
        void callEncoded(const QString& encoded, bool isConstant, int callIndex, const QVariantMap& userData) const;
        void callError(const QString& err) const;
        void newEvent(const EventInfo& info, bool isNew) const;
        void abiResult(const QString& abi) const;
        void busyChanged(bool busy) const;
        void callNameDone(const QString& name) const;
        void tokenBalanceDone(int accountIndex, const QString& tokenAddress, const QString& balance) const;
        void receivedTokens(const QString& value, const QString& token, const QString& sender) const;
    public slots:
        void reload();
        void onNewEvent(const QJsonObject& event, bool isNew, const QString& internalFilterID);
        void httpRequestDone(QNetworkReply *reply);
        void onCallDone(const QString& result, int index, const QVariantMap& userData);
        void onSelectedTokenContract(int index, bool forwardToAccounts = true);
        void onConfirmedTransaction(const QString &fromAddress, const QString& toAddress, const QString& hash);
        void onExistingAccountImported(const QString& address, int accountIndex);
    private:
        const QString getPostfix() const;
        void loadERC20Data(const ContractInfo& contract, int index) const;
        void onCallName(const QString& result) const;
        void refreshTokenBalance(const QString& accountAddress, int accountIndex, const ContractInfo& contract, int contractIndex) const;
        void onTokenBalance(const QString& result, int contractIndex, int accountIndex) const;
        void registerTokensFilter();
        const ContractInfo& getContractByAddress(const QString& address, int& index) const;

        ContractList fList;
        NodeIPC& fIpc;
        QNetworkAccessManager fNetManager;
        bool fBusy;
        PendingContracts fPendingContracts;
        AccountModel& fAccountModel;
        QMap<QString, bool> fTokenBalanceTabs;
    };

}

#endif // CONTRACTMODEL_H
