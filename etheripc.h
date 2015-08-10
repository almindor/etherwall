#ifndef ETHERIPC_H
#define ETHERIPC_H

#include <QLocalSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include "types.h"

namespace Etherwall {

    class EtherIPC
    {
    public:
        EtherIPC();
        void connect(const QString& path);

        const QJsonArray getAccountRefs();
        qulonglong getBalance(const QJsonValue& accountRef);
    private:
        QLocalSocket fSocket;
        int fCallNum;

        QJsonObject methodToJSON(const QString& method, const QJsonArray& params);
        const QJsonValue callIPC(const QString& method, const QJsonArray& params);
    };

}

#endif // ETHERIPC_H
