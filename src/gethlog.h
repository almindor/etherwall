#ifndef GETHLOG_H
#define GETHLOG_H

#include <QObject>
#include <QStringList>
#include <QAbstractListModel>
#include <QProcess>
#include "types.h"

namespace Etherwall {

    class GethLog : public QAbstractListModel
    {
        Q_OBJECT
    public:
        GethLog();

        QHash<int, QByteArray> roleNames() const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        Q_INVOKABLE void saveToClipboard() const;
        void attach(QProcess* process);
        void append(const QString& line);
    private:
        QStringList fList;
        QProcess* fProcess;
        void readStdout();
        void readStderr();
        void readData();
        void overflowCheck();
    };

}

#endif // GETHLOG_H
