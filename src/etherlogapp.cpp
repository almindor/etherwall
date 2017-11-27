#include "etherlogapp.h"
#include <QApplication>
#include <QClipboard>

namespace Etherwall {

    EtherLogApp::EtherLogApp() : EtherLog()
    {

    }

    void EtherLogApp::saveToClipboard() const
    {
         QApplication::clipboard()->setText(getContents());
    }

}
