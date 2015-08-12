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
#include <QLocalSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>
#include "types.h"

namespace Etherwall {

    class EtherIPC: public QObject
    {
        Q_OBJECT
    public:
        EtherIPC();
        void setWorker(QThread* worker);
    public slots:
        void connectToServer(const QString& path);
        void getAccounts();
        void newAccount(const QString& password, int index);
        void deleteAccount(const QString& hash, const QString& password, int index);
        void closeApp();
    signals:
        void connectToServerDone();
        void getAccountsDone(const AccountList& list);
        void newAccountDone(const QString& result, int index);
        void deleteAccountDone(bool result, int index);
        void error(const QString& error, int code);
    private:
        QLocalSocket fSocket;
        int fCallNum;
        QLocale fLocale;
        QString fError;
        int fCode;

        bool getAccountRefs(QJsonArray& result);
        bool getBalance(const QJsonValue& accountRef, QString& result, const QString& block = "latest");
        bool getTransactionCount(const QJsonValue& accountRef, quint64& result, const QString& block = "latest");

        QJsonObject methodToJSON(const QString& method, const QJsonArray& params);
        bool callIPC(const QString& method, const QJsonArray& params, QJsonValue& result);
    };

}

#endif // ETHERIPC_H

