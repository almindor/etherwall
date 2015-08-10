#include "accountmodel.h"
#include "types.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QStandardPaths>
#include <QSettings>

namespace Etherwall {

    AccountModel::AccountModel(QObject *parent) :
        QAbstractListModel(parent), fAccountList(), fIpc()
    {
        try {
#ifdef Q_OS_WIN32
            const QString defPath = "//.pipe/geth.ipc";
#else
            const QString defPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.ethereum/geth.ipc";
#endif
            const QSettings settings;
            const QString path = settings.value("ipc/path", defPath).toString();
            qDebug() << path << "\n";
            fIpc.connect(path);
            refresh();
        } catch ( const char* e ) {
            qDebug() << e << "\n"; // TODO: trow and handle properly
        }
    }

    QHash<int, QByteArray> AccountModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[HashRole] = "hash";
        roles[BalanceRole] = "balance";
        return roles;
    }

    int AccountModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const {
        return fAccountList.length();
    }

    QVariant AccountModel::data(const QModelIndex & index, int role) const {
        const int row = index.row();

        return fAccountList.at(row).value(role);
    }

    void AccountModel::refresh() {
        QJsonArray accRefs = fIpc.getAccountRefs();
        fAccountList.clear(); // after the call if we exceptioned out!

        foreach( QJsonValue r, accRefs ) {
            const QString hash = r.toString("INVALID");
            const qulonglong balance = fIpc.getBalance(r);

            fAccountList.append(AccountInfo(hash, balance));
        }
    }

}
