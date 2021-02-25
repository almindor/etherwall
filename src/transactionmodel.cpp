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
/** @file transactionmodel.cpp
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Transaction model implementation
 */

#include "transactionmodel.h"
#include "helpers.h"
#include "ethereum/tx.h"
#include <QDebug>
#include <QTimer>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QSettings>

namespace Etherwall {
    const int ALWAYS_FAILING_TX_ERROR = -32000;

    // we want to ensure that -32000 (always failing tx) will be handled nicely. Helps with ERC20s that throw if balance is insufficient
    bool handleGasEstimateError(int code, const QString& error, NodeRequestTypes requestType, QJsonValue& result) {
        Q_UNUSED(code) // filtered at source
        Q_UNUSED(error) // too unpredictable

        if ( requestType == EstimateGas ) { // if it's estimate gas
            result = QJsonValue("0x15F90"); // 90k gas default
            return true;
        }

        return false; // otherwise leave as error
    }

    TransactionModel::TransactionModel(NodeIPC& ipc, const AccountModel& accountModel, const QSslConfiguration& sslConfig) :
        QAbstractTableModel(nullptr), fSSLConfig(sslConfig), fIpc(ipc), fAccountModel(accountModel),
        fBlockNumber(0), fLastBlock(0), fFirstBlock(0), fGasPrice("0"), fGasEstimate("0"), fNetManager(this),
        fLatestVersion(QCoreApplication::applicationVersion())
    {
        ipc.registerIpcErrorHandler(ALWAYS_FAILING_TX_ERROR, &handleGasEstimateError);

        connect(&ipc, &NodeIPC::connectToServerDone, this, &TransactionModel::connectToServerDone);
        connect(&ipc, &NodeIPC::getAccountsDone, this, &TransactionModel::getAccountsDone);
        connect(&ipc, &NodeIPC::getBlockNumberDone, this, &TransactionModel::getBlockNumberDone);
        connect(&ipc, &NodeIPC::getGasPriceDone, this, &TransactionModel::getGasPriceDone);
        connect(&ipc, &NodeIPC::estimateGasDone, this, &TransactionModel::estimateGasDone);
        connect(&ipc, &NodeIPC::sendTransactionDone, this, &TransactionModel::onSendTransactionDone);
        connect(&ipc, &NodeIPC::signTransactionDone, this, &TransactionModel::onSignTransactionDone);
        connect(&ipc, &NodeIPC::newTransaction, this, &TransactionModel::onNewTransaction);
        connect(&ipc, &NodeIPC::newBlock, this, &TransactionModel::newBlock);
        connect(&ipc, &NodeIPC::syncingChanged, this, &TransactionModel::syncingChanged);

        connect(&fNetManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(httpRequestDone(QNetworkReply*)));
        checkVersion(); // TODO: move this off at some point
    }

    quint64 TransactionModel::getBlockNumber() const {
        return fBlockNumber;
    }

    quint64 TransactionModel::getFirstBlock() const {
        return fFirstBlock;
    }

    quint64 TransactionModel::getLastBlock() const {
        return fLastBlock;
    }

    const QString& TransactionModel::getGasPrice() const {
        return fGasPrice;
    }

    const QString& TransactionModel::getGasEstimate() const {
        return fGasEstimate;
    }

    const QString& TransactionModel::getLatestVersion() const {
        return fLatestVersion;
    }

    QHash<int, QByteArray> TransactionModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[Qt::DisplayRole] = "display";
        roles[THashRole] = "hash";
        roles[NonceRole] = "nonce";
        roles[SenderRole] = "sender";
        roles[ReceiverRole] = "receiver";
        roles[ValueRole] = "value";
        roles[BlockNumberRole] = "blocknumber";
        roles[BlockHashRole] = "blockhash";
        roles[TransactionIndexRole] = "tindex";
        roles[GasRole] = "gas";
        roles[GasPriceRole] = "gasprice";
        roles[InputRole] = "input";
        roles[DepthRole] = "depth";
        roles[SenderAliasRole] = "senderalias";
        roles[ReceiverAliasRole] = "receiveralias";

        return roles;
    }

    int TransactionModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const {
        return fTransactionList.size();
    }

    int TransactionModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 4;
    }

    QVariant TransactionModel::data(const QModelIndex & index, int role) const {
        const int row = index.row();

        if ( role == Qt::DisplayRole ) {
            switch (index.column()) {
                case 0: return fTransactionList.at(row).getBlockNumber();
                case 1: return fTransactionList.at(row).value(SenderAliasRole);
                case 2: return fTransactionList.at(row).value(ReceiverAliasRole);
                case 3: return fTransactionList.at(row).getValueFixed(2);
                // case 4: return fTransactionList.at(row).value(DepthRole);
            }

            return "?";
        }

        // calculate distance from current block
        if ( role == DepthRole ) {
            quint64 transBlockNum = fTransactionList.at(row).value(BlockNumberRole).toULongLong();
            if ( transBlockNum == 0 ) { // still pending
                return -1;
            }

            quint64 diff = fBlockNumber - transBlockNum;
            return diff;
        }

        return fTransactionList.at(row).value(role);
    }

    int TransactionModel::containsTransaction(const QString& hash) {
        int i = 0;
        foreach ( const TransactionInfo& t, fTransactionList ) {
            if ( t.value(THashRole).toString() == hash ) {
                return i;
            }
            i++;
        }

        return -1;
    }

    void TransactionModel::connectToServerDone() {
        fIpc.getBlockNumber();
        fIpc.getGasPrice();
    }

    void TransactionModel::getAccountsDone(const QStringList& list) {
        Q_UNUSED(list);
        refresh();
        loadHistory();
    }

    void TransactionModel::getBlockNumberDone(quint64 num) {
        if ( num <= fBlockNumber ) {
            return;
        }

        fBlockNumber = num;
        if ( fFirstBlock == 0 ) {
            fFirstBlock = num;
        }

        emit blockNumberChanged(num);

        if ( !fTransactionList.isEmpty() ) { // depth changed for all
            const QModelIndex& leftIndex = QAbstractTableModel::createIndex(0, 5);
            const QModelIndex& rightIndex = QAbstractTableModel::createIndex(fTransactionList.length() - 1, 5);
            QVector<int> roles(2);
            roles[0] = DepthRole;
            roles[1] = Qt::DisplayRole;
            emit dataChanged(leftIndex, rightIndex, roles);
        }
    }

    void TransactionModel::getGasPriceDone(const QString& num) {
        if ( num != fGasEstimate ) {
            fGasPrice = num;
            emit gasPriceChanged(num);
        }
    }

    void TransactionModel::estimateGasDone(const QString& num) {
        if ( num != fGasEstimate ) {
            fGasEstimate = num;
            emit gasEstimateChanged(num);
        }
    }

    void TransactionModel::sendTransaction(const QString& password, const QString& from, const QString& to,
                                           const QString& value, quint64 nonce, const QString& gas, const QString& gasPrice,
                                           const QString& data) {
        Ethereum::Tx tx(from, to, value, nonce, gas, gasPrice, data); // nonce not required here, ipc.sendTransaction doesn't fill it in as it's known to geth
        fQueuedTransaction.init(from, to, value, gas, gasPrice, data);

        if ( fIpc.isThinClient() ) {
            fIpc.signTransaction(tx, password);
        } else {
            fIpc.sendTransaction(tx, password);
        }
    }

    void TransactionModel::call(const QString &from, const QString &to, const QString &value, const QString &gas, const QString &gasPrice,
                               const QString &data, int index, const QVariantMap& userData)
    {
        Ethereum::Tx tx(from, to, value, 0, gas, gasPrice, data); // nonce not required here, ipc.call doesn't fill it in

        fIpc.call(tx, index, userData);
    }

    void TransactionModel::onRawTransaction(const Ethereum::Tx& tx)
    {
        fQueuedTransaction.init(tx.fromStr(), tx.toStr(), tx.valueStr(), tx.gasStr(), tx.gasPriceStr(), tx.dataStr());
        fIpc.sendRawTransaction(tx);
    }

    void TransactionModel::onSendTransactionDone(const QString& hash) {
        fQueuedTransaction.setHash(hash);
        addTransaction(fQueuedTransaction);
        storeTransaction(fQueuedTransaction);
        EtherLog::logMsg("Transaction sent, hash: " + hash);
    }

    void TransactionModel::onSignTransactionDone(const QString &hash)
    {
        fIpc.sendRawTransaction(hash);
    }

    void TransactionModel::onNewTransaction(const QJsonObject &json) {
        const TransactionInfo info(json);
        int ai1, ai2;
        const QString& sender = info.value(SenderRole).toString().toLower();
        const QString& receiver = info.value(ReceiverRole).toString().toLower();

        if ( fAccountModel.containsAccount(sender, receiver, ai1, ai2) ) { // either our sent or someone sent to us
            const int n = containsTransaction(info.value(THashRole).toString());
            if ( n >= 0 ) { // ours
                fTransactionList[n] = info;
                const QModelIndex& leftIndex = QAbstractTableModel::createIndex(n, 0);
                const QModelIndex& rightIndex = QAbstractTableModel::createIndex(n, 14);
                emit dataChanged(leftIndex, rightIndex);
                storeTransaction(fTransactionList.at(n));
            } else { // external from someone to us
                addTransaction(info);
                storeTransaction(info);
            }
        } else {
            addTransaction(info); // TODO handle tokens
        }
    }

    void TransactionModel::newBlock(const QJsonObject& block) {
        const QJsonArray transactions = block.value("transactions").toArray();
        const quint64 blockNum = Helpers::toQUInt64(block.value("number"));

        if ( blockNum == 0 ) {
            return; // not interested in pending blocks
        }

        fIpc.getGasPrice(); // let's update our gas price

        foreach ( QJsonValue t, transactions ) {
            const QJsonObject to = t.toObject();
            const QString thash = to.value("hash").toString();
            const QString sender = to.value("from").toString().toLower();
            const QString receiver = to.value("to").toString().toLower();
            int i1, i2;

            const int n = containsTransaction(thash);
            if ( n >= 0 ) {
                fTransactionList[n].init(to);
                const QModelIndex& leftIndex = QAbstractTableModel::createIndex(n, 0);
                const QModelIndex& rightIndex = QAbstractTableModel::createIndex(n, 14);
                QVector<int> roles(3);
                roles[0] = BlockNumberRole;
                roles[1] = DepthRole;
                roles[2] = Qt::DisplayRole;
                emit dataChanged(leftIndex, rightIndex, roles);
                const TransactionInfo info = fTransactionList.at(n);
                storeTransaction(fTransactionList.at(n));
                emit confirmedTransaction(info.getSender(), info.getReceiver(), info.getHash());
            } else if ( fAccountModel.containsAccount(sender, receiver, i1, i2) ) {
                const TransactionInfo info = TransactionInfo(to);
                addTransaction(info);
                storeTransaction(info);
                emit receivedTransaction(info.value(ReceiverRole).toString());
            }
        }
    }

    void TransactionModel::syncingChanged(bool syncing)
    {
        if ( !syncing ) {
            refresh();
        }
    }

    int TransactionModel::getInsertIndex(const TransactionInfo& info) const {
        const quint64 block = info.value(BlockNumberRole).toULongLong();

        if ( block == 0 ) {
            return 0; // new/pending
        }

        for ( int i = 0; i < fTransactionList.length(); i++ ) {
            const quint64 oldBlock = fTransactionList.at(i).value(BlockNumberRole).toULongLong();
            if ( oldBlock <= block ) {
                return i;
            }
        }

        return fTransactionList.length();
    }

    void TransactionModel::addTransaction(const TransactionInfo& info) {
        const int index = getInsertIndex(info);
        beginInsertRows(QModelIndex(), index, index);
        fTransactionList.insert(index, info);
        endInsertRows();
    }

    void TransactionModel::storeTransaction(const TransactionInfo& info) {
        // save to persistent memory for re-run
        const quint64 blockNum = info.value(BlockNumberRole).toULongLong();
        QSettings settings;
        settings.beginGroup("transactions");
        settings.setValue(Helpers::toDecStr(blockNum) + "_" + info.value(TransactionIndexRole).toString(), info.toJsonString());
        settings.endGroup();
    }

    bool transCompare(const TransactionInfo& a, const TransactionInfo& b) {
        return a.getBlockNumber() > b.getBlockNumber();
    }

    void TransactionModel::refresh()
    {
        QSettings settings;
        settings.beginGroup("transactions");
        QStringList list = settings.allKeys();

        foreach ( const QString bns, list ) {
            const QString val = settings.value(bns, "bogus").toString();
            if ( val.contains("{") ) { // new format, get data and reload only recent transactions
                QJsonParseError parseError;
                const QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8(), &parseError);

                if ( parseError.error != QJsonParseError::NoError ) {
                    EtherLog::logMsg("Error parsing stored transaction: " + parseError.errorString(), LS_Error);
                } else {
                    const QJsonObject json = jsonDoc.object();
                    quint64 txBlockNum = Helpers::toQUInt64(json.value("blockNumber"));

                    if ( txBlockNum > 0 ) { // don't add "pending", we might have a failed leftover
                        addTransaction(json);
                    } else {
                        settings.remove(bns);
                    }
                    // if transaction is newer than 1 day restore it from geth anyhow to ensure correctness in case of reorg
                    if ( txBlockNum == 0 || fBlockNumber - txBlockNum < 5400 ) {
                        fIpc.getTransactionByHash(json.value("hash").toString());
                    }
                }
            } else if ( val != "bogus" ) { // old format, re-get and store full data
                fIpc.getTransactionByHash(val);
                settings.remove(bns);
            }
        }
        settings.endGroup();

        std::sort(fTransactionList.begin(), fTransactionList.end(), transCompare);

        lookupAccountsAliases();
    }

    const QString TransactionModel::estimateTotal(const QString& value, const QString& gas, const QString& gasPrice) const {
        BigInt::Rossi valRossi = Helpers::etherStrToRossi(value);
        BigInt::Rossi valGas = Helpers::decStrToRossi(gas);
        BigInt::Rossi valGasPrice = Helpers::etherStrToRossi(gasPrice);

        const QString wei = QString((valRossi + valGas * valGasPrice).toStrDec().data());

        return Helpers::weiStrToEtherStr(wei);
    }

    const QString TransactionModel::getHash(int index) const {
        if ( index >= 0 && index < fTransactionList.length() ) {
            return Helpers::hexPrefix(fTransactionList.at(index).value(THashRole).toString());
        }

        return QString();
    }

    const QString TransactionModel::getSender(int index) const {
        if ( index >= 0 && index < fTransactionList.length() ) {
            return fTransactionList.at(index).value(SenderRole).toString();
        }

        return QString();
    }

    const QString TransactionModel::getReceiver(int index) const {
        if ( index >= 0 && index < fTransactionList.length() ) {
            return fTransactionList.at(index).value(ReceiverRole).toString();
        }

        return QString();
    }

    double TransactionModel::getValue(int index) const {
        if ( index >= 0 && index < fTransactionList.length() ) {
            return fTransactionList.at(index).value(ValueRole).toDouble();
        }

        return 0;
    }

    const QJsonObject TransactionModel::getJson(int index, bool decimal) const {
        if ( index < 0 || index >= fTransactionList.length() ) {
            return QJsonObject();
        }

        return fTransactionList.at(index).toJson(decimal);
    }

    const QString TransactionModel::getMaxValue(int row, const QString& gas, const QString& gasPrice) const {
        if ( row < 0 || row >= fAccountModel.size() ) {
            return "0";
        }
        const QModelIndex index = QAbstractTableModel::createIndex(row, 2);

        BigInt::Rossi balanceWeiRossi = Helpers::etherStrToRossi( fAccountModel.data(index, BalanceRole).toString() );
        const BigInt::Rossi gasRossi = Helpers::decStrToRossi(gas);
        const BigInt::Rossi gasPriceRossi = Helpers::etherStrToRossi(gasPrice);
        const BigInt::Rossi gasTotalRossi = gasRossi * gasPriceRossi;

        if ( balanceWeiRossi < gasTotalRossi ) {
            return "0";
        }
        const BigInt::Rossi resultWeiRossi = (balanceWeiRossi - gasTotalRossi);

        const QString resultWei = QString(resultWeiRossi.toStrDec().data());

        return Helpers::weiStrToEtherStr(resultWei);
    }

    void TransactionModel::lookupAccountsAliases() {
        for ( int n = 0; n < fTransactionList.size(); n++ ) {
            const QString sender = fTransactionList[n].getSender();
            const QString receiver = fTransactionList[n].getReceiver();

            fTransactionList[n].setSenderAlias(fAccountModel.getAccountAlias(sender));
            fTransactionList[n].setReceiverAlias(fAccountModel.getAccountAlias(receiver));
        }

        QVector<int> roles(3);
        roles[0] = SenderRole;
        roles[1] = ReceiverRole;
        roles[2] = Qt::DisplayRole;
        const QModelIndex& leftIndex = QAbstractTableModel::createIndex(0, 0);
        const QModelIndex& rightIndex = QAbstractTableModel::createIndex(fTransactionList.size(), 10);

        emit dataChanged(leftIndex, rightIndex, roles);
    }

    void TransactionModel::httpRequestDone(QNetworkReply *reply) {
        const QString uri = reply->url().fileName();

        if ( uri == "version" ) {
            checkVersionDone(reply);
        } else if ( uri == "transactions" ) {
            loadHistoryDone(reply);
        } else {
            EtherLog::logMsg("Unknown uri from reply: " + uri, LS_Error);
        }
    }

    void TransactionModel::checkVersion(bool manual) {
        // get latest app version
        QNetworkRequest request(QUrl("https://api.etherwall.com/api/version"));
        request.setSslConfiguration(fSSLConfig);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("x-internal-manual", manual ? "t" : "f");
        QJsonObject objectJson;
        const QByteArray data = QJsonDocument(objectJson).toJson();

        EtherLog::logMsg("HTTP Post request: " + data, LS_Debug);

        fNetManager.post(request, data);
    }

    void TransactionModel::checkVersionDone(QNetworkReply *reply) {
        QString err;
        auto parsed = Helpers::parseHTTPReply(reply, err);
        if ( !err.isEmpty() ) {
            emit error(err);
            return;
        }

        QJsonObject resObj = parsed.object();
        const bool success = resObj.value("success").toBool();

        if ( !success ) {
            const QString error = resObj.value("error").toString("unknown error");
            return EtherLog::logMsg("Response error: " + error, LS_Error);
        }
        bool manual = reply->request().rawHeader("x-internal-manual") == "t";
        const QJsonValue rv = resObj.value("result");
        fLatestVersion = rv.toString("0.0.0");
        int latestIntVer = Helpers::parseVersion(fLatestVersion);
        int intVer = Helpers::parseVersion(QCoreApplication::applicationVersion());

        if ( intVer < latestIntVer ) {
            emit latestVersionChanged(fLatestVersion, manual);
        } else {
            emit latestVersionSame(fLatestVersion, manual);
        }
    }

    void TransactionModel::loadHistory() {
        QSettings settings;
        if ( fAccountModel.rowCount() == 0 || fIpc.chainManager().testnet() ) {
            return; // don't try with empty request or on testnet
        }

        // get historical transactions from etherdata
        QNetworkRequest request(QUrl("https://api.etherwall.com/api/transactions"));
        request.setSslConfiguration(fSSLConfig);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QJsonObject objectJson;
        objectJson["accounts"] = fAccountModel.getAccountsJsonArray();
        const QByteArray data = QJsonDocument(objectJson).toJson();

        EtherLog::logMsg("HTTP Post request: " + data, LS_Debug);

        fNetManager.post(request, data);
    }

    void TransactionModel::loadHistoryDone(QNetworkReply *reply) {
        QString err;
        const auto parsed = Helpers::parseHTTPReply(reply, err);
        if ( !err.isEmpty() ) {
            emit error(err);
            return;
        }

        QJsonObject resObj = parsed.object();
        const bool success = resObj.value("success").toBool();

        if ( !success ) {
            const QString error = resObj.value("error").toString("unknown error");
            return EtherLog::logMsg("Response error: " + error, LS_Error);
        }
        const QJsonValue rv = resObj.value("result");
        const QJsonArray result = rv.toArray();

        int stored = 0;
        foreach ( const QJsonValue jv, result ) {
            const QJsonObject jo = jv.toObject();
            const QString hash = jo.value("hash").toString("bogus");
            if ( hash == "bogus" ) {
                return EtherLog::logMsg("Response hash missing", LS_Error);
            }

            if ( containsTransaction(hash) < 0 ) {
                const TransactionInfo info(jo);
                addTransaction(info);
                storeTransaction(info);
                stored++;
            }
        }

        if ( stored > 0 ) {
            EtherLog::logMsg("Restored " + QString::number(stored) + " transactions from etherdata server", LS_Info);
        }

        reply->close();
    }    

}
