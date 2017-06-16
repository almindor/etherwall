#ifndef REMOTEIPC_H
#define REMOTEIPC_H

#include <QtWebSockets/QtWebSockets>
#include <QObject>
#include "gethlog.h"
#include "etheripc.h"

namespace Etherwall {

    class RemoteIPC: public EtherIPC
    {
        Q_OBJECT
    public:
        RemoteIPC(const QString& ipcPath, GethLog& gethLog, const QString &remotePath);
        virtual ~RemoteIPC();
        Q_INVOKABLE virtual bool closeApp();
    protected slots:
        // override
        virtual void connectedToServer();
        virtual bool endpointWritable();
        virtual qint64 endpointWrite(const QByteArray& data);
        virtual const QByteArray endpointRead();

        void onConnectedWS();
        void onDisconnectedWS();
        void onErrorWS(QAbstractSocket::SocketError error);
        void onTextMessageReceivedWS(const QString& msg);
    private:
        QWebSocket fWebSocket;
        QByteArray fReceivedMessage;
        bool fIsThinClient;

        bool isRemoteRequest() const;
    };

}

#endif // REMOTEIPC_H
