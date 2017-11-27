#include "gethlogapp.h"
#include <QApplication>
#include <QClipboard>

namespace Etherwall {

    GethLogApp::GethLogApp() : GethLog()
    {

    }

    void GethLogApp::saveToClipboard() const
    {
         QApplication::clipboard()->setText(getContents());
    }

}
