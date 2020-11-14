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
/** @file accountmodel.h
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Account model header
 */

#ifndef ACCOUNTMODEL_H
#define ACCOUNTMODEL_H

#include <QAbstractTableModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QMap>
#include <QUrl>
#include "types.h"
#include "currencymodel.h"
#include "nodeipc.h"
#include "etherlog.h"
#include "trezor/trezor.h"

namespace Etherwall {

    class AccountModel : public QAbstractTableModel
    {
        Q_OBJECT
        Q_PROPERTY(int selectedAccountRow READ getSelectedAccountRow WRITE setSelectedAccountRow NOTIFY accountSelectionChanged)
        Q_PROPERTY(QString selectedAccount READ getSelectedAccount NOTIFY accountSelectionChanged)
        Q_PROPERTY(bool selectedAccountDefault READ getSelectedAccountDefault NOTIFY accountSelectionChanged)
        Q_PROPERTY(QString selectedAccountAlias READ getSelectedAccountAlias NOTIFY accountSelectionChanged)
        Q_PROPERTY(QString selectedAccountBalance READ getSelectedAccountBalance NOTIFY accountSelectionChanged)
        Q_PROPERTY(QString selectedAccountDeviceID READ getSelectedAccountDeviceID NOTIFY accountSelectionChanged)
        Q_PROPERTY(QString selectedAccountHDPath READ getSelectedAccountHDPath NOTIFY accountSelectionChanged)
        Q_PROPERTY(quint64 selectedAccountSentTrans READ getSelectedAccountSentTrans NOTIFY accountSelectionChanged)
        Q_PROPERTY(QString total READ getTotal NOTIFY totalChanged)
        Q_PROPERTY(bool busy MEMBER fBusy NOTIFY busyChanged)
        Q_PROPERTY(int defaultIndex READ getDefaultIndex NOTIFY defaultIndexChanged)
        Q_PROPERTY(QString currentToken READ getCurrentToken NOTIFY currentTokenChanged)
    public:
        AccountModel(NodeIPC& ipc, const CurrencyModel& currencyModel, Trezor::TrezorDevice& trezor);
        QString getError() const;
        QHash<int, QByteArray> roleNames() const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        bool containsAccount(const QString& from, const QString& to, int& i1, int& i2) const;
        const QJsonArray getAccountsJsonArray() const;
        const QString getAccountAlias(const QString& hash) const;
        const QString getTotal() const;
        int size() const;
        void refreshAccounts();
        const QString getSelectedAccountAlias() const;
        const QString getSelectedAccountBalance() const;
        quint64 getSelectedAccountSentTrans() const;
        const QString getSelectedAccountDeviceID() const;
        const QString getSelectedAccountHDPath() const;
        bool getSelectedAccountDefault() const;
        const AccountList& getAccounts() const;
        const QVariantList getAccountAddresses() const;
        int getAccountIndex(const QString& address) const;
        void selectToken(const QString& name, const QString& tokenAddress);

        Q_INVOKABLE void newAccount(const QString& pw);
        Q_INVOKABLE void renameAccount(const QString& name, int index);
        Q_INVOKABLE void removeAccounts();
        Q_INVOKABLE void removeAccount(const QString& address);
        Q_INVOKABLE const QString getAccountHash(int index) const;
        Q_INVOKABLE const QString getAccountHDPath(int index) const;
        Q_INVOKABLE quint64 getAccountNonce(int index) const;
        Q_INVOKABLE void exportWallet(const QUrl& fileName) const;
        Q_INVOKABLE void importWallet(const QUrl& fileName);
        Q_INVOKABLE bool exportAccount(const QUrl& fileName, int index);
        Q_INVOKABLE void setAsDefault(const QString& address);
        Q_INVOKABLE void trezorImport(quint32 offset, quint8 count);
        Q_INVOKABLE const QString getMaxTokenValue(int accountIndex, const QString& tokenAddress) const;
    public slots:
        void onTokenBalanceDone(int accountIndex, const QString& tokenAddress, const QString& balance);
    private slots:
        void connectToServerDone();
        void getAccountsDone(const QStringList& list);
        void newAccountDone(const QString& hash, int index);
        void accountBalanceChanged(int index, const QString& balanceStr);
        void accountSentTransChanged(int index, quint64 count);
        void newBlock(const QJsonObject& block);
        void currencyChanged();
        void syncingChanged(bool syncing);
        void importWalletDone();
        void onTrezorInitialized(const QString& deviceID);
        void onTrezorAddressRetrieved(const QString& address, const QString& hdPath);
    signals:
        void accountsReady() const;
        void accountSelectionChanged(int) const;
        void totalChanged() const;
        void walletErrorEvent(const QString& error) const;
        void walletExportedEvent() const;
        void walletImportedEvent() const;
        void busyChanged(bool busy) const;
        void defaultIndexChanged(int index) const;
        void promptForTrezorImport() const;
        void accountsRemoved() const;
        void currentTokenChanged() const;
        void existingAccountImported(const QString& address, int accountIndex) const;
    private:
        NodeIPC& fIpc;
        AccountList fAccountList;
        QMap<QString, QString> fAliasMap;
        Trezor::TrezorDevice& fTrezor;
        int fSelectedAccountRow;
        QString fSelectedAccount;
        const CurrencyModel& fCurrencyModel;
        bool fBusy;
        QString fCurrentToken;
        QString fCurrentTokenAddress;

        int getSelectedAccountRow() const;
        int getDefaultIndex() const;
        bool hasDefaultIndex() const;
        void setSelectedAccountRow(int row);
        const QString getSelectedAccount() const;
        void storeAccountList() const;
        void loadAccountList();
        void setAccountAlias(const QString& hash, const QString& alias);
        int exportableAddresses() const;
        const QString getCurrentToken() const;
    };

}

#endif // ACCOUNTMODEL_H
