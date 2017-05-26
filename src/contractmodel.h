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
#include "contractinfo.h"
#include "etheripc.h"

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
        ContractModel(EtherIPC& ipc);

        QHash<int, QByteArray> roleNames() const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        Q_INVOKABLE bool addContract(const QString& name, const QString& address, const QString& abi);
        Q_INVOKABLE bool addPendingContract(const QString& name, const QString& abi, const QString& hash);
        Q_INVOKABLE const QString contractDeployed(const QJsonObject& receipt);
        Q_INVOKABLE bool deleteContract(int index);
        Q_INVOKABLE const QString getName(int index) const;
        Q_INVOKABLE int getIndex(const QString name) const;
        Q_INVOKABLE const QString getAddress(int index) const;
        Q_INVOKABLE const QString getABI(int index) const;
        Q_INVOKABLE const QStringList getFunctions(int index) const;
        Q_INVOKABLE const QString getMethodID(int index, const QString& functionName) const;
        Q_INVOKABLE const QVariantList getArguments(int index, const QString& functionName) const;
        Q_INVOKABLE void encodeCall(int index, const QString& functionName, const QVariantList& params);
        Q_INVOKABLE void requestAbi(const QString& address);
    signals:
        void callEncoded(const QString& encoded) const;
        void callError(const QString& err) const;
        void newEvent(const EventInfo& info, bool isNew) const;
        void abiResult(const QString& abi) const;
        void busyChanged(bool busy) const;
    public slots:
        void reload();
        void onNewEvent(const QJsonObject& event, bool isNew);
        void httpRequestDone(QNetworkReply *reply);
    private:
        const QString getPostfix() const;

        ContractList fList;
        EtherIPC& fIpc;
        QNetworkAccessManager fNetManager;
        bool fBusy;
        PendingContracts fPendingContracts;
    };

}

#endif // CONTRACTMODEL_H
