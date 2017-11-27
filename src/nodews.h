#ifndef REMOTEIPC_H
#define REMOTEIPC_H

#include <QtWebSockets/QtWebSockets>
#include <QObject>
#include "gethlog.h"
#include "nodeipc.h"

namespace Etherwall {

    class NodeWS: public NodeIPC
    {
        Q_OBJECT
        Q_PROPERTY(bool thinClient READ isThinClient NOTIFY thinClientChanged)
    public:
        NodeWS(GethLog& gethLog);
        virtual ~NodeWS();
        virtual void getLogs(const QStringList& addresses, const QJsonArray& topics, quint64 fromBlock, const QString& internalID);
        virtual bool isThinClient() const;

        Q_INVOKABLE virtual bool closeApp();
        Q_INVOKABLE virtual void setInterval(int interval);
    public slots:
        virtual void start(const QString &version, const QString &endpoint, const QString &warning);
    protected slots:
        // override
        virtual void finishInit();
        virtual bool endpointWritable();
        virtual qint64 endpointWrite(const QByteArray& data);
        virtual const QByteArray endpointRead();
        virtual const QStringList buildGethArgs();

        void onConnectedWS();
        void onDisconnectedWS();
        void onErrorWS(QAbstractSocket::SocketError error);
        void onTextMessageReceivedWS(const QString& msg);
    signals:
        void thinClientChanged();
    private:
        QWebSocket fWebSocket;
        QString fEndpoint;
        QByteArray fReceivedMessage;
        bool fIsThinClient;

        bool isRemoteRequest() const;
        void connectWebsocket();
    };

}

#endif // REMOTEIPC_H
