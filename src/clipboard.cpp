#include "clipboard.h"

namespace Etherwall {

    ClipboardAdapter::ClipboardAdapter(QObject *parent) : QObject(parent) {
        fClipboard = QApplication::clipboard();
    }

    void ClipboardAdapter::setText(QString text) {
        fClipboard->setText(text, QClipboard::Clipboard);
        fClipboard->setText(text, QClipboard::Selection);
    }

}
