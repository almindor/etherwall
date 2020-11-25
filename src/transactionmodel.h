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
/** @file transactionmodel.h
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Transaction model header
 */


#ifndef TRANSACTIONMODEL_H
#define TRANSACTIONMODEL_H


#include <QAbstractTableModel>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "types.h"
#include "nodeipc.h"
#include "accountmodel.h"
#include "etherlog.h"

namespace Etherwall {

    class TransactionModel : public QAbstractTableModel
    {
        Q_OBJECT
        Q_PROPERTY(quint64 firstBlock READ getFirstBlock NOTIFY blockNumberChanged)
        Q_PROPERTY(quint64 lastBlock READ getLastBlock NOTIFY blockNumberChanged)
        Q_PROPERTY(quint64 blockNumber READ getBlockNumber NOTIFY blockNumberChanged FINAL)
        Q_PROPERTY(QString gasPrice READ getGasPrice NOTIFY gasPriceChanged FINAL)
        Q_PROPERTY(QString gasEstimate READ getGasEstimate NOTIFY gasEstimateChanged FINAL)
        Q_PROPERTY(QString latestVersion READ getLatestVersion NOTIFY latestVersionChanged FINAL)
    public:
        TransactionModel(NodeIPC& ipc, const AccountModel& accountModel, const QSslConfiguration& sslConfig);
        quint64 getBlockNumber() const;
        const QString& getGasPrice() const;
        const QString& getLatestVersion() const;
        const QString& getGasEstimate() const;
        QHash<int, QByteArray> roleNames() const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        Q_INVOKABLE virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        int containsTransaction(const QString& hash);

        Q_INVOKABLE void sendTransaction(const QString& password, const QString& from, const QString& to,
                             const QString& value, quint64 nonce, const QString& gas = QString(),
                             const QString& gasPrice = QString(), const QString& data = QString());

        Q_INVOKABLE void call(const QString& from, const QString& to,
                             const QString& value, const QString& gas,
                             const QString& gasPrice, const QString& data,
                             int index, const QVariantMap& userData);

        Q_INVOKABLE const QString estimateTotal(const QString& value, const QString& gas, const QString& gasPrice) const;
        Q_INVOKABLE void loadHistory();
        Q_INVOKABLE const QString getHash(int index) const;
        Q_INVOKABLE const QString getSender(int index) const;
        Q_INVOKABLE const QString getReceiver(int index) const;
        Q_INVOKABLE double getValue(int index) const;
        Q_INVOKABLE const QJsonObject getJson(int index, bool decimal) const;
        Q_INVOKABLE const QString getMaxValue(int row, const QString& gas, const QString& gasPrice) const;
        Q_INVOKABLE void lookupAccountsAliases();
        Q_INVOKABLE void checkVersion(bool manual = false);
        double getHistoryProgress() const;
        quint64 getFirstBlock() const;
        quint64 getLastBlock() const;
    public slots:
        void onRawTransaction(const Ethereum::Tx& tx);
    private slots:
        void connectToServerDone();
        void getAccountsDone(const QStringList& list);
        void getBlockNumberDone(quint64 num);
        void getGasPriceDone(const QString& num);
        void estimateGasDone(const QString& num);
        void onSendTransactionDone(const QString& hash);
        void onSignTransactionDone(const QString& hash);
        void onNewTransaction(const QJsonObject& json);
        void newBlock(const QJsonObject& block);
        void syncingChanged(bool syncing);
        void refresh();
        void loadHistoryDone(QNetworkReply* reply);
        void checkVersionDone(QNetworkReply *reply);
        void httpRequestDone(QNetworkReply *reply);
    signals:
        void error(const QString& error) const;
        void blockNumberChanged(quint64 num) const;
        void gasPriceChanged(const QString& price) const;
        void gasEstimateChanged(const QString& price) const;
        void historyChanged() const;
        void latestVersionChanged(const QString& version, bool manualVersionCheck) const;
        void latestVersionSame(const QString& version, bool manualVersionCheck) const;
        void receivedTransaction(const QString& toAddress) const;
        void confirmedTransaction(const QString& fromAddress, const QString& toAddress, const QString& hash) const;
    private:
        const QSslConfiguration fSSLConfig;
        NodeIPC& fIpc;
        const AccountModel& fAccountModel;
        TransactionList fTransactionList;
        quint64 fBlockNumber;
        quint64 fLastBlock;
        quint64 fFirstBlock;
        QString fGasPrice;
        QString fGasEstimate;
        TransactionInfo fQueuedTransaction;
        QNetworkAccessManager fNetManager;
        QString fLatestVersion;

        int getInsertIndex(const TransactionInfo& info) const;
        void addTransaction(const TransactionInfo& info);
        void storeTransaction(const TransactionInfo& info);
        void refreshPendingTransactions();
    };

}


#endif // TRANSACTIONMODEL_H
