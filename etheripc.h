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

#include <QLocalSocket>
#include <QJsonObject>
#include <QJsonArray>
#include "types.h"

namespace Etherwall {

    class EtherIPC
    {
    public:
        EtherIPC();
        void connect(const QString& path);
        const QString& getError() const;

        const QJsonArray getAccountRefs();
        const QString getBalance(const QJsonValue& accountRef, const QString& block = "latest");
        quint64 getTransactionCount(const QJsonValue& accountRef, const QString& block = "latest");
        const QString newAccount(const QString& password);
        bool deleteAccount(const QString& hash, const QString& password);
    private:
        QLocalSocket fSocket;
        int fCallNum;
        QLocale fLocale;
        QString fError;

        QJsonObject methodToJSON(const QString& method, const QJsonArray& params);
        const QJsonValue callIPC(const QString& method, const QJsonArray& params);
    };

}

#endif // ETHERIPC_H

