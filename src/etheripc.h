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
/** @file etheripc.h
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Ethereum IPC client header
 */

#ifndef ETHERIPC_H
#define ETHERIPC_H

#include <QObject>
#include <QList>
#include <QLocalSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>
#include "types.h"

namespace Etherwall {

    class RequestIPC {
    public:
        RequestIPC(RequestTypes type, const QString method, const QJsonArray params = QJsonArray(), int index = -1);
        RequestIPC(bool empty);
        RequestIPC();

        RequestTypes getType() const;
        const QString& getMethod() const;
        const QJsonArray& getParams() const;
        int getIndex() const;
        int getCallID() const;
        bool empty() const;
        static int sCallID;
    private:
        int fCallID;
        RequestTypes fType;
        QString fMethod;
        QJsonArray fParams;
        int fIndex;
        bool fEmpty;
    };

    typedef QList<RequestIPC> RequestList;

    class EtherIPC: public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString error READ getError NOTIFY error)
        Q_PROPERTY(int code READ getCode NOTIFY error)
        Q_PROPERTY(bool busy READ getBusy NOTIFY busyChanged)
        Q_PROPERTY(int connectionState READ getConnectionState NOTIFY connectionStateChanged)
        Q_PROPERTY(quint64 peerCount READ peerCount NOTIFY peerCountChanged)
        Q_PROPERTY(const QString connectionStateStr READ getConnectionStateStr NOTIFY connectionStateChanged)
    public:
        EtherIPC();
        void setWorker(QThread* worker);
        bool getBusy() const;
        const QString& getError() const;
        int getCode() const;
    public slots:
        void connectToServer(const QString& path);
        void connectedToServer();
        void disconnectedFromServer();
        void getAccounts();
        void newAccount(const QString& password, int index);
        void deleteAccount(const QString& hash, const QString& password, int index);
        void getBlockNumber();
        void getPeerCount();
        void sendTransaction(const QString& from, const QString& to, double value);
        void unlockAccount(const QString& hash, const QString& password, int duration, int index);
        void getGasPrice();
        void onSocketReadyRead();
        void onSocketError(QLocalSocket::LocalSocketError err);
        void closeApp();
    signals:
        void connectToServerDone();
        void getAccountsDone(const AccountList& list);
        void newAccountDone(const QString& result, int index);
        void deleteAccountDone(bool result, int index);
        void getBlockNumberDone(quint64 num);
        void sendTransactionDone(const QString& hash);
        void unlockAccountDone(bool result, int index);
        void getGasPriceDone(const QString& price);

        void peerCountChanged(quint64 num);
        void busyChanged(bool busy);
        void connectionStateChanged();
        void error(const QString& error, int code);
    private:
        QLocalSocket fSocket;
        QLocale fLocale;
        QString fError;
        int fCode;
        quint64 fPeerCount;
        QString fPath;
        AccountList fAccountList;
        RequestList fRequestQueue;
        quint64 fPendingTransactionsFilterID;
        RequestIPC fActiveRequest;

        void handleNewAccount();
        void handleDeleteAccount();
        void handleAccountDetails();
        void handleAccountBalance();
        void handleAccountTransactionCount();
        void handleGetBlockNumber();
        void handleGetPeerCount();
        void handleSendTransaction();
        void handleUnlockAccount();
        void handleGetGasPrice();
        void handleNewPendingTransactionFilter();

        int getConnectionState() const;
        const QString getConnectionStateStr() const;
        quint64 peerCount() const;
        void bail();
        void done();
        void newPendingTransactionFilter();

        QJsonObject methodToJSON(const RequestIPC& request);
        bool queueRequest(const RequestIPC& request);
        bool writeRequest();
        bool readReply(QJsonValue& result);
        bool readNumber(quint64& result);
        const QString toDecStr(const QJsonValue& jv) const;
    };

}

#endif // ETHERIPC_H

