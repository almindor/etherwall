#include "remoteipc.h"
#include <QUrl>

namespace Etherwall {

    RemoteIPC::RemoteIPC(const QString& ipcPath, GethLog& gethLog, const QString &remotePath) :
        EtherIPC(ipcPath, gethLog),
        fWebSocket("http://localhost"), fReceivedMessage()
    {
        QObject::connect(&fWebSocket, &QWebSocket::disconnected, this, &RemoteIPC::onDisconnectedWS);
        QObject::connect(&fWebSocket, &QWebSocket::connected, this, &RemoteIPC::onConnectedWS);
        QObject::connect(&fWebSocket, (void (QWebSocket::*)(QAbstractSocket::SocketError))&QWebSocket::error, this, &RemoteIPC::onErrorWS);
        QObject::connect(&fWebSocket, &QWebSocket::textMessageReceived, this, &RemoteIPC::onTextMessageReceivedWS);

        const QSettings settings;
        fIsThinClient = settings.value("geth/thinclient", false).toBool();

        if ( fIsThinClient ) {
            fWebSocket.open(QUrl(remotePath));
        }
    }

    RemoteIPC::~RemoteIPC()
    {
        fWebSocket.close(); // in case we missed the app closing
    }

    bool RemoteIPC::closeApp()
    {
        bool result = EtherIPC::closeApp();

        // wait for websocket if we're still not disconnected (only after all others are done tho!)
        if ( result && fWebSocket.state() != QAbstractSocket::UnconnectedState ) {
            fWebSocket.close();
            return false;
        }

        return result;
    }

    void RemoteIPC::connectedToServer()
    {
        // if we're in IPC mode
        if ( !fIsThinClient ) {
            EtherIPC::connectedToServer();
        }

        // if we're relady connected when ipc is done just let it continue, otherwise wait for WS
        if ( fWebSocket.state() == QAbstractSocket::ConnectedState ) {
            EtherIPC::connectedToServer();
        }
    }

    bool RemoteIPC::endpointWritable()
    {
        if ( isRemoteRequest() ) {
            return true;
        }

        return EtherIPC::endpointWritable();
    }

    qint64 RemoteIPC::endpointWrite(const QByteArray &data)
    {
        if ( isRemoteRequest() ) {
            qint64 sent = fWebSocket.sendBinaryMessage(data);
            qDebug() << "WS SEND[" << sent << "]: " << data << "\n";
            return sent;
        }

        return EtherIPC::endpointWrite(data);
    }

    const QByteArray RemoteIPC::endpointRead()
    {
        if ( isRemoteRequest() ) {
            qDebug() << "WS Reading\n";
            const QByteArray result = fReceivedMessage;
            fReceivedMessage.clear(); // ensure we get empties if this gets called out of order
            return result;
        }

        return EtherIPC::endpointRead();
    }

    void RemoteIPC::onConnectedWS()
    {
        qDebug() << "WS Connected\n";
        // if IPC is connected at this stage continue with init
        if ( fSocket.state() == QLocalSocket::ConnectedState ) {
            EtherIPC::connectedToServer();
        }
    }

    void RemoteIPC::onDisconnectedWS()
    {
        if ( !fClosingApp ) {
            setError("WS: Disconnected from websocket");
            bail();
        }
    }

    void RemoteIPC::onErrorWS(QAbstractSocket::SocketError error)
    {
        Q_UNUSED(error);
        setError("WS: " + fSocket.errorString());
        bail();
    }

    void RemoteIPC::onTextMessageReceivedWS(const QString &msg)
    {
        qDebug() << "WS GOT: " << msg << "\n";
        fReceivedMessage = msg.toUtf8();
        onSocketReadyRead();
    }

    bool RemoteIPC::isRemoteRequest() const
    {
        if ( !fIsThinClient ) {
            return false; // all are considered local in this case
        }

        switch ( fActiveRequest.getType() ) {
            case NoRequest: return false;
            case NewAccount: return false;
            case GetAccountRefs: return false;
            case SendTransaction: return false;
            case GetClientVersion: return false;
            case GetNetVersion: return false;
            case GetSyncing: return false;
            default: return true;
        }
    }

}
