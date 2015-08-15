#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QApplication>
#include <QClipboard>
#include <QObject>

namespace Etherwall {

    class ClipboardAdapter : public QObject
    {
        Q_OBJECT
    public:
        explicit ClipboardAdapter(QObject *parent = 0);

        Q_INVOKABLE void setText(QString text);
    private:
        QClipboard* fClipboard;
    };

}

#endif // CLIPBOARD_H
