#ifndef REMOTEIPC_H
#define REMOTEIPC_H

#include <QObject>
#include "gethlog.h"
#include "etheripc.h"

namespace Etherwall {

    class RemoteIPC: public EtherIPC
    {
        Q_OBJECT
    public:
        RemoteIPC(const QString& baseURL, GethLog& gethLog);
    private:
        QString fBaseURL;
    };

}

#endif // REMOTEIPC_H
