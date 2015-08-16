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
        QAbstractListModel(0), fIpc(ipc), fAccountModel(accountModel), fBlockNumber(0), fGasPrice("unknown")
    {
        connect(&ipc, &EtherIPC::connectToServerDone, this, &TransactionModel::connectToServerDone);
        connect(&ipc, &EtherIPC::getBlockNumberDone, this, &TransactionModel::getBlockNumberDone);
        connect(&ipc, &EtherIPC::getGasPriceDone, this, &TransactionModel::getGasPriceDone);
        connect(&ipc, &EtherIPC::sendTransactionDone, this, &TransactionModel::sendTransactionDone);
        connect(&ipc, &EtherIPC::newPendingTransaction, this, &TransactionModel::newTransaction);
    }

    quint64 TransactionModel::getBlockNumber() const {
        return fBlockNumber;
    }

    const QString& TransactionModel::getGasPrice() const {
        return fGasPrice;
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

        return roles;
    }

    int TransactionModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const {
        return fTransactionList.length();
    }

    QVariant TransactionModel::data(const QModelIndex & index, int role) const {
        const int row = index.row();

        return fTransactionList.at(row).value(role);
    }

    void TransactionModel::connectToServerDone() {
        fIpc.getBlockNumber();
        fIpc.getGasPrice();
    }

    void TransactionModel::getTransactionsDone(const TransactionList &list) {
        beginResetModel();
        fTransactionList = list;
        endResetModel();
    }

    void TransactionModel::getBlockNumberDone(quint64 num) {
        fBlockNumber = num;
        emit blockNumberChanged(num);
    }

    void TransactionModel::getGasPriceDone(const QString& num) {
        fGasPrice = num;
        emit gasPriceChanged(num);
    }

    void TransactionModel::sendTransaction(const QString& from, const QString& to, double value) {
        fIpc.sendTransaction(from, to, value);
    }

    void TransactionModel::sendTransactionDone(const QString& hash) {
        qDebug() << "Transaction sent hash: " << hash << "\n";
    }

    void TransactionModel::newTransaction(const TransactionInfo &info) {
        int ai1, ai2;
        if ( fAccountModel.containsAccount(info, ai1, ai2) ) {
            const int size = fTransactionList.length();
            beginInsertRows(QModelIndex(), size, size);
            fTransactionList.append(info);
            endInsertRows();

            // handle via block filter!
            /*
            if ( ai1 >= 0 ) {
                const QString& hash1 = fAccountModel.getAccountHash(ai1);
                fIpc.refreshAccount(hash1, ai1);
            }

            if ( ai2 >= 0 ) {
                const QString& hash2 = fAccountModel.getAccountHash(ai2);
                fIpc.refreshAccount(hash2, ai2);
            }*/

        }
    }

}
