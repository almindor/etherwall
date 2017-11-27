#ifndef GETHLOGAPP_H
#define GETHLOGAPP_H

#include "gethlog.h"

namespace Etherwall {

    class GethLogApp: public GethLog
    {
        Q_OBJECT
    public:
        GethLogApp();

        Q_INVOKABLE void saveToClipboard() const;
    };
}

#endif // GETHLOGAPP_H
