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

        RequestTypes getType() const;
        const QString& getMethod() const;
        const QJsonArray& getParams() const;
        int getIndex() const;
    private:
        RequestTypes fType;
        QString fMethod;
        QJsonArray fParams;
        int fIndex;
    };

    typedef QList<RequestIPC> RequestList;

    class EtherIPC: public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString error READ getError NOTIFY error)
        Q_PROPERTY(int code READ getCode NOTIFY error)
        Q_PROPERTY(bool busy READ getBusy NOTIFY busyChanged)
    public:
        EtherIPC();
        void setWorker(QThread* worker);
        bool getBusy() const;
        const QString& getError() const;
        int getCode() const;
        void start(const QString& ipcPath);
    public slots:
        void connectToServer(const QString& path);
        void getAccounts();
        void newAccount(const QString& password, int index);
        void deleteAccount(const QString& hash, const QString& password, int index);
        void getBlockNumber();
        void onSocketReadyRead();
        void onSocketError(QLocalSocket::LocalSocketError err);
        void closeApp();
    signals:
        void connectToServerDone();
        void getAccountsDone(const AccountList& list);
        void newAccountDone(const QString& result, int index);
        void deleteAccountDone(bool result, int index);
        void getBlockNumberDone(quint64 num);
        void busyChanged(bool busy);
        void error(const QString& error, int code);
    private:
        QLocalSocket fSocket;
        int fCallNum;
        QLocale fLocale;
        QString fError;
        int fCode;
        bool fBusy;
        AccountList fAccountList;
        RequestList fRequestQueue;

        void handleNewAccount();
        void handleDeleteAccount();
        void handleGetBlockNumber();
        void handleAccountDetails();
        void handleAccountBalance();
        void handleAccountTransactionCount();

        void bail();
        void done();
        int index() const;
        RequestTypes requestType() const;

        QJsonObject methodToJSON(const RequestIPC& request);
        bool writeRequest(const RequestIPC& request, bool fromQueue = false);
        bool readReply(QJsonValue& result);
    };

}

#endif // ETHERIPC_H

