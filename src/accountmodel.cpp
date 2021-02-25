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
/** @file accountmodel.cpp
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Account model implementation
 */

#include "accountmodel.h"
#include "helpers.h"
#include "trezor/hdpath.h"
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>

#define EMPTY_BALANCE "0.000000000000000000"
#define DEFAULT_DEVICE "geth"

namespace Etherwall {

    AccountModel::AccountModel(NodeIPC& ipc, const CurrencyModel& currencyModel, Trezor::TrezorDevice& trezor) :
        QAbstractTableModel(0),
        fIpc(ipc), fAccountList(), fAliasMap(), fTrezor(trezor),
        fSelectedAccountRow(-1), fCurrencyModel(currencyModel), fBusy(false),
        fCurrentToken("ETH")
    {
        connect(&ipc, &NodeIPC::connectToServerDone, this, &AccountModel::connectToServerDone);
        connect(&ipc, &NodeIPC::getAccountsDone, this, &AccountModel::getAccountsDone);
        connect(&ipc, &NodeIPC::newAccountDone, this, &AccountModel::newAccountDone);
        connect(&ipc, &NodeIPC::accountBalanceChanged, this, &AccountModel::accountBalanceChanged);
        connect(&ipc, &NodeIPC::accountSentTransChanged, this, &AccountModel::accountSentTransChanged);
        connect(&ipc, &NodeIPC::newBlock, this, &AccountModel::newBlock);
        connect(&ipc, &NodeIPC::syncingChanged, this, &AccountModel::syncingChanged);

        connect(&currencyModel, &CurrencyModel::currencyChanged, this, &AccountModel::currencyChanged);

        connect(&trezor, &Trezor::TrezorDevice::initialized, this, &AccountModel::onTrezorInitialized);
        connect(&trezor, &Trezor::TrezorDevice::addressRetrieved, this, &AccountModel::onTrezorAddressRetrieved);
    }

    QHash<int, QByteArray> AccountModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[Qt::DisplayRole] = "display";
        roles[HashRole] = "hash";
        roles[DefaultRole] = "default";
        roles[BalanceRole] = "balance";
        roles[TransCountRole] = "transactions";
        roles[SummaryRole] = "summary";
        roles[AliasRole] = "alias";
        roles[DeviceRole] = "device";
        roles[DeviceTypeRole] = "deviceType";

        return roles;
    }

    int AccountModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const {
        return fAccountList.size();
    }

    int AccountModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 5;
    }

    QVariant AccountModel::data(const QModelIndex & index, int role) const {
        const int row = index.row();

        if ( role == Qt::DisplayRole ) {
            switch ( index.column() ) {
                case 0: return fAccountList.at(row).value(DefaultRole);
                case 1: return fAccountList.at(row).value(DeviceTypeRole);
                case 2: return fAccountList.at(row).alias();
                case 3: return fAccountList.at(row).hash();
                case 4: return fAccountList.at(row).getBalanceFixed(2);
            }

            return "?";
        } else {
            QVariant result = fAccountList.at(row).value(role);
            if ( role == BalanceRole ) {
                return fCurrencyModel.recalculate(result);
            }

            return result;
        }
    }

    // TODO: optimize with hashmap
    bool AccountModel::containsAccount(const QString& from, const QString& to, int& i1, int& i2) const {
        i1 = -1;
        i2 = -1;
        int i = 0;
        foreach ( const AccountInfo& a, fAccountList ) {
            const QString addr = a.value(HashRole).toString().toLower();
            if ( addr == from ) {
                i1 = i;
            }

            if ( addr == to ) {
                i2 = i;
            }
            i++;
        }

        return (i1 >= 0 || i2 >= 0);
    }

    const QString AccountModel::getTotal() const {
        BigInt::Rossi total;

        foreach ( const AccountInfo& info, fAccountList ) {
            const QVariant balance = info.value(BalanceRole);
            double dVal = fCurrencyModel.recalculate(balance).toDouble();
            const QString strVal = QString::number(dVal, 'f', 18);
            total += Helpers::etherStrToRossi(strVal);
        }

        const QString weiStr = QString(total.toStrDec().data());
        return Helpers::weiStrToEtherStr(weiStr);
    }

    int AccountModel::size() const
    {
        return fAccountList.size();
    }

    void AccountModel::newAccount(const QString& pw) {
        const int index = fAccountList.size();
        fIpc.newAccount(pw, index);
    }

    void AccountModel::renameAccount(const QString& name, int index) {
        if ( index >= 0 && index < fAccountList.size() ) {
            fAccountList[index].setAlias(name);
            setAccountAlias(fAccountList.at(index).hash(), name);

            QVector<int> roles(3);
            roles[0] = AliasRole;
            roles[1] = SummaryRole;
            roles[2] = Qt::DisplayRole;
            const QModelIndex& leftIndex = QAbstractTableModel::createIndex(index, 0);
            const QModelIndex& rightIndex = QAbstractTableModel::createIndex(index, 5);

            storeAccountList();
            emit dataChanged(leftIndex, rightIndex, roles);
        } else {
            EtherLog::logMsg("Invalid account selection for rename", LS_Error);
        }
    }

    void AccountModel::removeAccounts()
    {
        beginResetModel();
        foreach ( const AccountInfo& info, fAccountList ) {
            if ( !info.HDPath().isEmpty() ) {
                removeAccount(info.hash());
            }
        }
        endResetModel();

        emit accountsRemoved();
    }

    void AccountModel::removeAccount(const QString& address) {
        QSettings settings;
        const QString key = address.toLower();

        beginResetModel();

        const QString chain = fIpc.chainManager().networkPostfix();
        settings.beginGroup("accounts" + chain);
        settings.remove(key);
        settings.endGroup();

        for ( int i = 0; i < fAccountList.size(); i++ ) {
            if ( fAccountList.at(i).hash().toLower() == key ) {
                fAccountList.removeAt(i);
                break;
            }
        }

        endResetModel();
    }

    const QString AccountModel::getAccountHash(int index) const {
        if ( index >= 0 && fAccountList.length() > index ) {
            return fAccountList.at(index).value(HashRole).toString();
        }

        return QString();
    }

    const QString AccountModel::getAccountHDPath(int index) const
    {
        if ( index >= 0 && fAccountList.length() > index ) {
            return fAccountList.at(index).HDPath();
        }

        return QString();
    }

    quint64 AccountModel::getAccountNonce(int index) const
    {
        if ( index >= 0 && fAccountList.length() > index ) {
            return fAccountList.at(index).transactionCount() + fIpc.nonceStart();
        }

        return 0; // TODO: throw
    }

    bool AccountModel::exportAccount(const QUrl& dir, int index) {
        if ( index < 0 || index >= fAccountList.length() ) {
            EtherLog::logMsg("No account to export", LS_Error);
            return false;
        }

        const QSettings settings;
        QDir keystore(settings.value("geth/datadir").toString());
        if ( fIpc.chainManager().testnet() ) {
            keystore.cd("rinkeby");
        }
        keystore.cd("keystore");

        try {
            const QString address = fAccountList.at(index).value(HashRole).toString();
            const QString data = Helpers::exportAddress(keystore, address);
            const QString fileName = Helpers::getAddressFilename(keystore, address);
            QDir directory(dir.toLocalFile());
            QFile file(directory.absoluteFilePath(fileName));
            if ( !file.open(QFile::WriteOnly) ) {
                EtherLog::logMsg("Account export file error: " + file.errorString(), LS_Error);
                return false;
            }
            QTextStream stream( &file );
            stream << data;
            file.close();

            return true;
        } catch ( QString err ) {
            EtherLog::logMsg("Error on account export: " + err, LS_Error);
            return false;
        }
    }

    void AccountModel::setAsDefault(const QString &address)
    {
        const int prevIndex = getDefaultIndex();
        QSettings settings;
        const QString defaultKey = "default/" + fIpc.chainManager().networkPostfix();
        settings.beginGroup("accounts");
        settings.setValue(defaultKey, address.toLower());
        settings.endGroup();

        const int newIndex = getDefaultIndex();
        emit defaultIndexChanged(newIndex);
        const QModelIndex& leftIndex = QAbstractTableModel::createIndex(prevIndex, 0);
        const QModelIndex& rightIndex = QAbstractTableModel::createIndex(prevIndex, 0);
        emit dataChanged(leftIndex, rightIndex);

        {
            const QModelIndex& leftIndex = QAbstractTableModel::createIndex(newIndex, 0);
            const QModelIndex& rightIndex = QAbstractTableModel::createIndex(newIndex, 0);
            emit dataChanged(leftIndex, rightIndex);
        }
    }

    void AccountModel::trezorImport(quint32 offset, quint8 count)
    {
        const QString hdPathBase = fIpc.chainManager().hdPathBase();
        quint32 total = offset + count;

        for ( quint32 i = offset; i < total; i++ ) {
            const QString fullPath = hdPathBase + "/" + QString::number(i);
            const Trezor::HDPath hdPath(fullPath);
            fTrezor.getAddress(hdPath);
        }
    }

    const QString AccountModel::getMaxTokenValue(int accountIndex, const QString &tokenAddress) const
    {
        if ( accountIndex < 0 || accountIndex >= fAccountList.size() ) {
            return "0";
        }

        return fAccountList.at(accountIndex).getTokenBalance(tokenAddress);
    }

    void AccountModel::onTokenBalanceDone(int accountIndex, const QString &tokenAddress, const QString &balance)
    {
        if ( accountIndex < 0 || accountIndex >= fAccountList.size() ) {
            return EtherLog::logMsg("Invalid account index on token balance", LS_Error);
        }

        if ( tokenAddress.isEmpty() || balance.isEmpty() ) {
            return EtherLog::logMsg("Token address or balance invalid", LS_Error);
        }

        fAccountList[accountIndex].setTokenBalance(tokenAddress, balance);

        if ( fAccountList.at(accountIndex).getCurrentTokenAddress() == tokenAddress ) {
            QVector<int> roles(2);
            roles[0] = TokenBalanceRole;
            roles[1] = Qt::DisplayRole;
            const QModelIndex& leftIndex = QAbstractTableModel::createIndex(accountIndex, 0);
            const QModelIndex& rightIndex = QAbstractTableModel::createIndex(accountIndex, 5);
            emit dataChanged(leftIndex, rightIndex, roles);
            emit totalChanged();
        }
    }

    const QString AccountModel::getSelectedAccountAlias() const
    {
        if ( fSelectedAccountRow < 0 || fSelectedAccountRow >= fAccountList.size() ) {
            return QString();
        }

        return fAccountList.at(fSelectedAccountRow).alias();
    }

    const QString AccountModel::getSelectedAccountBalance() const
    {
        if ( fSelectedAccountRow < 0 || fSelectedAccountRow >= fAccountList.size() ) {
            return QString();
        }

        return fAccountList.at(fSelectedAccountRow).value(BalanceRole).toString();
    }

    quint64 AccountModel::getSelectedAccountSentTrans() const
    {
        if ( fSelectedAccountRow < 0 || fSelectedAccountRow >= fAccountList.size() ) {
            return 0;
        }

        return fAccountList.at(fSelectedAccountRow).transactionCount();
    }

    const QString AccountModel::getSelectedAccountDeviceID() const
    {
        if ( fSelectedAccountRow < 0 || fSelectedAccountRow >= fAccountList.size() ) {
            return QString();
        }

        return fAccountList.at(fSelectedAccountRow).deviceID();
    }

    const QString AccountModel::getSelectedAccountHDPath() const
    {
        if ( fSelectedAccountRow < 0 || fSelectedAccountRow >= fAccountList.size() ) {
            return 0;
        }

        return fAccountList.at(fSelectedAccountRow).HDPath();
    }

    bool AccountModel::getSelectedAccountDefault() const
    {
        if ( fSelectedAccountRow < 0 || fSelectedAccountRow >= fAccountList.size() ) {
            return false;
        }

        return fSelectedAccountRow == getDefaultIndex();
    }

    const AccountList &AccountModel::getAccounts() const
    {
        return fAccountList;
    }

    const QVariantList AccountModel::getAccountAddresses() const
    {
        QVariantList result;
        foreach ( const AccountInfo& account, fAccountList ) {
            result.append(account.hash()); // TODO: check if we might need lowercase here
        }

        return result;
    }

    int AccountModel::getAccountIndex(const QString &address) const
    {
        const QString addressLower = address.toLower();
        for ( int i = 0; i < fAccountList.size(); i++ ) {
            if ( fAccountList.at(i).hash().toLower() == addressLower ) {
                return i;
            }
        }

        throw QString("Account not found");
    }

    void AccountModel::selectToken(const QString& name, const QString& tokenAddress)
    {
        fCurrentToken = name;
        fCurrentTokenAddress = tokenAddress;
        beginResetModel();
        for ( int i = 0; i < fAccountList.size(); i++ ) {
            fAccountList[i].setCurrentTokenAddress(tokenAddress);
        }
        endResetModel();

        emit currentTokenChanged();
        emit totalChanged();
    }

    void AccountModel::exportWallet(const QUrl& fileName) const {
        const QSettings settings;
        QDir keystore(settings.value("geth/datadir").toString());
        keystore.cd("keystore");

        try {
            int exported = 0;
            QByteArray backupData = Helpers::createBackup(keystore, exported);
            if ( !fIpc.chainManager().testnet() ) { // only do counts check on non-testnet
                int expected = exportableAddresses();
                if ( exported != expected ) {
                    throw QString("Export account count mismatch, expected " + QString::number(expected) + " exported " + QString::number(exported));
                }
            }
            QString strName = fileName.toLocalFile();
            const QFileInfo fileInfo(strName);
            if ( fileInfo.completeSuffix() != "etherwall" ) {
                strName += ".etherwall"; // force suffix
            }

            QFile file(strName);
            if ( !file.open(QFile::WriteOnly) ) {
                throw file.errorString();
            }
            if ( file.write(backupData) < backupData.size() ) {
                throw file.errorString();
            }
            file.close();
        } catch ( QString err ) {
            EtherLog::logMsg("Wallet export error: " + err, LS_Error);
            emit walletErrorEvent(err);
            return;
        }

        emit walletExportedEvent();
    }

    void AccountModel::importWallet(const QUrl& fileName) {
        const QSettings settings;
        try {
            QDir keystore(settings.value("geth/datadir").toString());
            keystore.cd("keystore");
            QFile file(fileName.toLocalFile());
            if ( !file.exists() ) {
                throw QString("Wallet backup file not found");
            }
            if ( !file.open(QFile::ReadOnly) ) {
                throw file.errorString();
            }
            QByteArray backupData = file.readAll();
            Helpers::restoreBackup(backupData, keystore);
            file.close();
        } catch ( QString err ) {
            EtherLog::logMsg("Wallet import error: " + err, LS_Error);
            emit walletErrorEvent(err);
            return;
        }

        fBusy = true;
        emit busyChanged(true);
        QTimer::singleShot(2000, this, SLOT(importWalletDone()));
    }

    void AccountModel::importWalletDone() {
        fBusy = false;
        emit busyChanged(false);
        fIpc.getAccounts();
        emit walletImportedEvent();
    }

    void AccountModel::onTrezorInitialized(const QString &deviceID)
    {
        foreach ( const AccountInfo& addr, fAccountList ) {
            if ( deviceID == addr.deviceID() ) {
                return; // we have some addresses don't prompt for them
            }
        }

        emit promptForTrezorImport();
    }

    void AccountModel::onTrezorAddressRetrieved(const QString &address, const QString& hdPath)
    {
        int i1, i2;
        if ( !containsAccount(address, "unused", i1, i2) ) {
            beginInsertRows(QModelIndex(), fAccountList.size(), fAccountList.size());
            fAccountList.append(AccountInfo(address, QString(), fTrezor.getDeviceID(), EMPTY_BALANCE, 0, hdPath, fIpc.chainManager().chainID()));
            fAccountList.last().setCurrentTokenAddress(fCurrentTokenAddress);
            endInsertRows();
            fIpc.refreshAccount(address, fAccountList.size() - 1); // refresh ETH
            emit existingAccountImported(Helpers::vitalizeAddress(address), fAccountList.size() - 1); // refresh ERC20 (all), NOTE: needs to be vitalized!

            storeAccountList();
        } else if ( fAccountList.at(i1).deviceID() != fTrezor.getDeviceID() ) { // this shouldn't happen unless they reimported to another hd device
            fAccountList[i1].setDeviceID(fTrezor.getDeviceID());

            QVector<int> roles(2);
            roles[0] = TokenBalanceRole;
            roles[1] = DeviceRole;
            const QModelIndex& leftIndex = QAbstractTableModel::createIndex(i1, i1);
            const QModelIndex& rightIndex = QAbstractTableModel::createIndex(i1, i1);
            emit dataChanged(leftIndex, rightIndex, roles);
        }
    }

    void AccountModel::connectToServerDone() {
        loadAccountList();
        fIpc.getAccounts();
    }

    void AccountModel::newAccountDone(const QString& hash, int index) {
        if ( !hash.isEmpty() ) {
            beginInsertRows(QModelIndex(), index, index);
            fAccountList.append(AccountInfo(hash, QString(), DEFAULT_DEVICE, EMPTY_BALANCE, 0, QString(), fIpc.chainManager().chainID()));
            fAccountList.last().setCurrentTokenAddress(fCurrentTokenAddress);
            endInsertRows();
            EtherLog::logMsg("New account created");

            if (fAccountList.size() > 0 && !hasDefaultIndex()) {
                setAsDefault(fAccountList.at(0).hash());
            }
        } else {
            EtherLog::logMsg("Account create failure");
        }
    }

    void AccountModel::currencyChanged() {
        QVector<int> roles(2);
        roles[0] = BalanceRole;
        roles[1] = Qt::DisplayRole;

        const QModelIndex& leftIndex = QAbstractTableModel::createIndex(0, 0);
        const QModelIndex& rightIndex = QAbstractTableModel::createIndex(fAccountList.size() - 1, 4);
        emit dataChanged(leftIndex, rightIndex, roles);
        emit totalChanged();
    }

    void AccountModel::syncingChanged(bool syncing) {
        if ( !syncing ) {
            refreshAccounts();
        }
    }

    void AccountModel::getAccountsDone(const QStringList& list) {
        beginResetModel();
        foreach ( const QString& addr, list ) {
            int i1, i2;
            if ( !containsAccount(addr, "unused", i1, i2) ) {
                fAccountList.append(AccountInfo(addr, QString(), DEFAULT_DEVICE, EMPTY_BALANCE, 0, QString(), fIpc.chainManager().chainID()));
            }
        }
        // drop non-hw accounts removed from geth somehow
        for ( int i = fAccountList.size() - 1; i >= 0; i-- ) {
            const AccountInfo& info = fAccountList.at(i);
            if ( !info.HDPath().isEmpty() ) {
                continue; // keep hw accounts
            }

            if ( !list.contains(info.hash(), Qt::CaseInsensitive) ) {
                fAccountList.removeAt(i);
            }
        }
        endResetModel();

        storeAccountList();

        refreshAccounts();

        if (fAccountList.size() > 0 && !hasDefaultIndex()) {
            setAsDefault(fAccountList.at(0).hash());
        }

        emit accountsReady();
        emit defaultIndexChanged(getDefaultIndex());
    }

    void AccountModel::refreshAccounts() {
        int i = 0;
        foreach ( const AccountInfo& info, fAccountList ) {
            const QString& hash = info.value(HashRole).toString();
            fIpc.refreshAccount(hash, i++);
        }

        emit totalChanged();
    }

    void AccountModel::accountBalanceChanged(int index, const QString& balanceStr) {
        if ( fAccountList.size() <= index ) {
            qDebug() << "Invalid index\n";
            return;
        }

        fAccountList[index].setBalance(balanceStr);
        const QModelIndex& leftIndex = QAbstractTableModel::createIndex(index, 0);
        const QModelIndex& rightIndex = QAbstractTableModel::createIndex(index, 4);
        emit dataChanged(leftIndex, rightIndex);
        emit totalChanged();
    }

    void AccountModel::accountSentTransChanged(int index, quint64 count) {
        if ( fAccountList.size() <= index ) {
            qDebug() << "Invalid index\n";
            return;
        }

        fAccountList[index].setTransactionCount(count);
        const QModelIndex& leftIndex = QAbstractTableModel::createIndex(index, 0);
        const QModelIndex& rightIndex = QAbstractTableModel::createIndex(index, 4);
        emit dataChanged(leftIndex, rightIndex);
    }

    void AccountModel::newBlock(const QJsonObject& block) {
        const QJsonArray transactions = block.value("transactions").toArray();
        const QString miner = block.value("miner").toString("bogus").toLower();
        int i1, i2;
        if ( containsAccount(miner, "bogus", i1, i2) ) {
            fIpc.refreshAccount(miner, i1);
        }

        foreach ( QJsonValue t, transactions ) {
            const QJsonObject to = t.toObject();
            const TransactionInfo info(to);
            const QString& sender = info.value(SenderRole).toString().toLower();
            const QString& receiver = info.value(ReceiverRole).toString().toLower();

            if ( containsAccount(sender, receiver, i1, i2) ) {
                if ( i1 >= 0 ) {
                    fIpc.refreshAccount(sender, i1);
                }

                if ( i2 >= 0 ) {
                    fIpc.refreshAccount(receiver, i2);
                }
            }
        }

        emit totalChanged();
    }

    int AccountModel::getSelectedAccountRow() const {
        return fSelectedAccountRow;
    }

    int AccountModel::getDefaultIndex() const
    {
        const QSettings settings;
        const QString defaultKey = "accounts/default/" + fIpc.chainManager().networkPostfix();
        const QString address = settings.value(defaultKey).toString().toLower();

        if ( address.isEmpty() ) {
            return 0;
        }

        for ( int i = 0; i < fAccountList.size(); i++ ) {
            if ( fAccountList.at(i).hash().toLower() == address ) {
                return i;
            }
        }

        return 0;
    }

    bool AccountModel::hasDefaultIndex() const
    {
        const QSettings settings;
        const QString defaultKey = "accounts/default/" + fIpc.chainManager().networkPostfix();
        const QString address = settings.value(defaultKey).toString();
        if ( address.isEmpty() ) {
            return false;
        }

        foreach ( const AccountInfo info, fAccountList ) {
            if ( info.hash().toLower() == address ) {
                return true;
            }
        }

        return false;
    }

    void AccountModel::setSelectedAccountRow(int row) {
        fSelectedAccountRow = row;
        emit accountSelectionChanged(row);
    }

    const QString AccountModel::getSelectedAccount() const {
        return getAccountHash(fSelectedAccountRow);
    }

    void AccountModel::storeAccountList() const
    {
        QSettings settings;

        const QString chain = fIpc.chainManager().networkPostfix();
        settings.beginGroup("accounts" + chain);
        int index = 0;
        foreach ( const AccountInfo& addr, fAccountList ) {
            const QString key = addr.hash().toLower();

            QJsonObject json = addr.toJson();
            json["index"] = index++;
            const QJsonDocument doc(json);
            const QString serialized = doc.toJson(QJsonDocument::Compact);

            settings.setValue(key, serialized);
        }
        settings.endGroup();
    }

    void AccountModel::loadAccountList()
    {
        QSettings settings;

        const QString chain = fIpc.chainManager().networkPostfix();
        settings.beginGroup("accounts" + chain);
        const QStringList keys = settings.allKeys();
        QList<QJsonObject> parsedList;

        foreach ( const QString& key, keys ) {
            const QString serialized = settings.value(key, "invalid").toString();
            parsedList.append(QJsonDocument::fromJson(serialized.toUtf8()).object());
        }
        settings.endGroup();

        // SORT parsed list based on index
        std::sort(parsedList.begin(), parsedList.end(), [](const QJsonObject& a, const QJsonObject& b) {
            return a.value("index").toInt() < b.value("index").toInt();
        });

        foreach ( const QJsonObject json, parsedList ) {
            const QString hash = json.value("hash").toString();
            const QString alias = json.value("alias").toString();
            const QString deviceID = json.value("deviceID").toString();
            const QString hdPath = json.value("HDPath").toString();
            fAccountList.append(AccountInfo(hash, alias, deviceID, EMPTY_BALANCE, 0, hdPath, fIpc.chainManager().chainID()));
            setAccountAlias(hash, alias);
        }
    }

    void AccountModel::setAccountAlias(const QString &hash, const QString &alias)
    {
        fAliasMap[hash.toLower()] = alias;
    }

    int AccountModel::exportableAddresses() const
    {
        int count = 0;
        foreach ( const AccountInfo& info, fAccountList ) {
            if ( info.isLocal() ) {
                count++;
            }
        }

        return count;
    }

    const QString AccountModel::getCurrentToken() const
    {
        return fCurrentToken;
    }

    const QJsonArray AccountModel::getAccountsJsonArray() const {
        QJsonArray result;

        foreach ( const AccountInfo ai, fAccountList ) {
            const QString hash = ai.value(HashRole).toString();
            result.append(hash);
        }

        return result;
    }

    const QString AccountModel::getAccountAlias(const QString &hash) const
    {
        return fAliasMap.value(hash.toLower(), QString());
    }

}
