#ifndef ETHERLOGAPP_H
#define ETHERLOGAPP_H

#include "etherlog.h"

namespace Etherwall {

    class EtherLogApp: public EtherLog
    {
        Q_OBJECT
    public:
        EtherLogApp();

        Q_INVOKABLE void saveToClipboard() const;
    };
}


#endif // ETHERLOGAPP_H
