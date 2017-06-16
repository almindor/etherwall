#include "remoteipc.h"

namespace Etherwall {

    RemoteIPC::RemoteIPC(const QString &baseURL, GethLog& gethLog) :
        EtherIPC(QString(), gethLog),
        fBaseURL(baseURL)
    {
    }

}
