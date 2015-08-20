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
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QSettings>

namespace Etherwall {

    TransactionModel::TransactionModel(EtherIPC& ipc, const AccountModel& accountModel) :
        QAbstractListModel(0), fIpc(ipc), fAccountModel(accountModel), fBlockNumber(0), fGasPrice("unknown"), fGasEstimate("unknown")
    {
        connect(&ipc, &EtherIPC::connectToServerDone, this, &TransactionModel::connectToServerDone);
        connect(&ipc, &EtherIPC::getAccountsDone, this, &TransactionModel::getAccountsDone);
        connect(&ipc, &EtherIPC::getBlockNumberDone, this, &TransactionModel::getBlockNumberDone);
        connect(&ipc, &EtherIPC::getGasPriceDone, this, &TransactionModel::getGasPriceDone);
        connect(&ipc, &EtherIPC::estimateGasDone, this, &TransactionModel::estimateGasDone);
        connect(&ipc, &EtherIPC::sendTransactionDone, this, &TransactionModel::sendTransactionDone);
        connect(&ipc, &EtherIPC::newTransaction, this, &TransactionModel::newTransaction);
        connect(&ipc, &EtherIPC::newBlock, this, &TransactionModel::newBlock);
    }

    quint64 TransactionModel::getBlockNumber() const {
        return fBlockNumber;
    }

    const QString& TransactionModel::getGasPrice() const {
        return fGasPrice;
    }

    const QString& TransactionModel::getGasEstimate() const {
        return fGasEstimate;
    }

    QHash<int, QByteArray> TransactionModel::roleNames() const {
        QHash<int, QByteArray> roles;
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

        return roles;
    }

    int TransactionModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const {
        return fTransactionList.size();
    }

    QVariant TransactionModel::data(const QModelIndex & index, int role) const {
        const int row = index.row();

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

    void TransactionModel::getAccountsDone(const AccountList& list __attribute__((unused))) {
        refresh();
    }

    void TransactionModel::getBlockNumberDone(quint64 num) {
        if ( num <= fBlockNumber ) {
            return;
        }

        fBlockNumber = num;
        emit blockNumberChanged(num);

        if ( !fTransactionList.isEmpty() ) { // depth changed for all
            const QModelIndex& leftIndex = QAbstractListModel::createIndex(0, 5);
            const QModelIndex& rightIndex = QAbstractListModel::createIndex(fTransactionList.length() - 1, 5);
            emit dataChanged(leftIndex, rightIndex, QVector<int>(1, DepthRole));
        }
    }

    void TransactionModel::getGasPriceDone(const QString& num) {
        fGasPrice = num;
        emit gasPriceChanged(num);
    }

    void TransactionModel::estimateGasDone(const QString& num) {
        fGasEstimate = num;
        emit gasEstimateChanged(num);
    }

    void TransactionModel::sendTransaction(const QString& from, const QString& to, double value, double gas) {
        fIpc.sendTransaction(from, to, value, gas);
        fQueuedTransaction.init(from, to, value, gas);
    }

    void TransactionModel::sendTransactionDone(const QString& hash) {
        fQueuedTransaction.setHash(hash);
        qDebug() << "sent hash: " << hash << "\n";
        addTransaction(fQueuedTransaction);
    }

    void TransactionModel::newTransaction(const TransactionInfo &info) {
        int ai1, ai2;
        const QString& sender = info.value(SenderRole).toString();
        const QString& receiver = info.value(ReceiverRole).toString();
        if ( fAccountModel.containsAccount(sender, receiver, ai1, ai2) ) { // either our sent or someone sent to us
            const int n = containsTransaction(info.value(THashRole).toString());
            if ( n >= 0 ) { // ours
                fTransactionList[n] = info;
                const QModelIndex& leftIndex = QAbstractListModel::createIndex(n, 0);
                const QModelIndex& rightIndex = QAbstractListModel::createIndex(n, 12);
                emit dataChanged(leftIndex, rightIndex);
                storeTransaction(fTransactionList.at(n));
            } else { // external from someone to us
                addTransaction(info);
            }
        }
    }

    void TransactionModel::newBlock(const QJsonObject& block) {
        const QJsonArray transactions = block.value("transactions").toArray();
        const quint64 blockNum = Helpers::toQUInt64(block.value("number"));

        if ( blockNum == 0 ) {
            return; // not interested in pending blocks
        }

        foreach ( QJsonValue t, transactions ) {
            const QJsonObject to = t.toObject();
            const QString thash = to.value("hash").toString();
            const QString sender = to.value("from").toString();
            const QString receiver = to.value("to").toString();
            int i1, i2;

            const int n = containsTransaction(thash);
            if ( n >= 0 ) {
                fTransactionList[n].setBlockNumber(blockNum);
                const QModelIndex& leftIndex = QAbstractListModel::createIndex(n, 0);
                const QModelIndex& rightIndex = QAbstractListModel::createIndex(n, 12);
                QVector<int> roles(2);
                roles[0] = BlockNumberRole;
                roles[1] = DepthRole;
                emit dataChanged(leftIndex, rightIndex, roles);
                storeTransaction(fTransactionList.at(n));
            } else if ( fAccountModel.containsAccount(sender, receiver, i1, i2) ) {
                addTransaction(TransactionInfo(to));
            }
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

        storeTransaction(info);
    }

    void TransactionModel::storeTransaction(const TransactionInfo& info) {
        // save to persistent memory for re-run
        const QString hash = info.value(THashRole).toString();
        const quint64 blockNum = info.value(BlockNumberRole).toULongLong();
        QSettings settings;
        settings.beginGroup("transactions");
        settings.setValue(Helpers::toDecStr(blockNum), hash);
        settings.endGroup();
    }

    void TransactionModel::refresh() {
        QSettings settings;
        settings.beginGroup("transactions");
        QStringList list = settings.allKeys();

        foreach ( const QString bns, list ) {
            const QString hash = settings.value(bns, "bogus").toString();
            if ( hash != "bogus" ) {
                fIpc.getTransactionByHash(hash);
            }
        }
        settings.endGroup();
    }

    void TransactionModel::loadHistory() {

    }

}
