#ifndef ETHERLOG_H
#define ETHERLOG_H

#include <QObject>
#include <QAbstractListModel>
#include "types.h"

namespace Etherwall {

    class EtherLog : public QAbstractListModel
    {
        Q_OBJECT
        Q_PROPERTY(int logLevel READ getLogLevel WRITE setLogLevel NOTIFY logLevelChanged)
    public:
        EtherLog();

        QHash<int, QByteArray> roleNames() const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        static void logMsg(const QString& msg, LogSeverity sev = LS_Info);
        Q_INVOKABLE void saveToClipboard() const;
        Q_INVOKABLE void log(QString msg, LogSeverity sev = LS_Info);
        int getLogLevel() const;
        void setLogLevel(int ll);
    signals:
        void logLevelChanged();
    public slots:
    private:
        LogList fList;
        LogSeverity fLogLevel;
    };

}

#endif // ETHERLOG_H
