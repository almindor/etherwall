#include "nodews.h"
#include "helpers.h"
#include <QUrl>

namespace Etherwall {

    NodeWS::NodeWS(GethLog& gethLog) :
        NodeIPC(gethLog),
        fWebSocket("http://localhost"), fEndpoint(), fReceivedMessage()
    {
        QObject::connect(&fWebSocket, &QWebSocket::disconnected, this, &NodeWS::onDisconnectedWS);
        QObject::connect(&fWebSocket, &QWebSocket::connected, this, &NodeWS::onConnectedWS);
        QObject::connect(&fWebSocket, (void (QWebSocket::*)(QAbstractSocket::SocketError))&QWebSocket::error, this, &NodeWS::onErrorWS);
        QObject::connect(&fWebSocket, &QWebSocket::textMessageReceived, this, &NodeWS::onTextMessageReceivedWS);

        const QSettings settings;
        fIsThinClient = settings.value("geth/thinclient", true).toBool();
    }

    NodeWS::~NodeWS()
    {
        fWebSocket.close(); // in case we missed the app closing
    }

    void NodeWS::getLogs(const QStringList &addresses, const QJsonArray &topics, quint64 fromBlock, const QString& internalID)
    {
        if ( !fIsThinClient ) {
            return NodeIPC::getLogs(addresses, topics, fromBlock, internalID);
        }

        // do nothing on remote, getlogs is too expensive we just don't support it on a thin client
    }

    bool NodeWS::closeApp()
    {
        bool result = NodeIPC::closeApp();

        // wait for websocket if we're still not disconnected (only after all others are done tho!)
        if ( result && fWebSocket.state() != QAbstractSocket::UnconnectedState ) {
            fWebSocket.close();
            return false;
        }

        return result;
    }

    void NodeWS::setInterval(int interval)
    {
        Q_UNUSED(interval); // remote enforced to 10s
        fTimer.setInterval(10000);
    }

    void NodeWS::start(const QString &version, const QString &endpoint, const QString &warning)
    {
        Q_UNUSED(version); // TODO
        Q_UNUSED(warning);

        const QSettings settings; // reinit because of first time dialog
        fIsThinClient = settings.value("geth/thinclient", true).toBool();
        fEndpoint = endpoint;

        connectWebsocket();
        NodeIPC::start(version, endpoint, warning);
    }

    void NodeWS::finishInit()
    {
        // hold off until WS is done too if remote
        if ( !fIsThinClient || fWebSocket.state() == QAbstractSocket::ConnectedState ) {
            NodeIPC::finishInit();
        }
    }

    bool NodeWS::endpointWritable()
    {
        if ( isRemoteRequest() ) {
            return true;
        }

        return NodeIPC::endpointWritable();
    }

    qint64 NodeWS::endpointWrite(const QByteArray &data)
    {
        if ( isRemoteRequest() ) {
            qint64 sent = fWebSocket.sendBinaryMessage(data);
            return sent;
        }

        return NodeIPC::endpointWrite(data);
    }

    const QByteArray NodeWS::endpointRead()
    {
        if ( isRemoteRequest() ) {
            const QByteArray result = fReceivedMessage;
            fReceivedMessage.clear(); // ensure we get empties if this gets called out of order
            return result;
        }

        return NodeIPC::endpointRead();
    }

    const QStringList NodeWS::buildGethArgs()
    {
        QStringList args = NodeIPC::buildGethArgs();
        if ( fIsThinClient ) {
            args.append("--maxpeers=0");
            args.append("--nodiscover");
            args.append("--nat=none");
        }

        return args;
    }

    void NodeWS::onConnectedWS()
    {
        EtherLog::logMsg("Connected to WS endpoint", LS_Info);
        // if IPC is connected at this stage continue with init
        if ( fSocket.state() == QLocalSocket::ConnectedState ) {
            NodeIPC::finishInit();
        }
    }

    void NodeWS::onDisconnectedWS()
    {
        if ( !fClosingApp ) {
            setError("WS: Disconnected from websocket");
            bail();
        }
    }

    void NodeWS::onErrorWS(QAbstractSocket::SocketError error)
    {
        Q_UNUSED(error);
        setError("WS: " + fSocket.errorString());
        bail();
    }

    void NodeWS::onTextMessageReceivedWS(const QString &msg)
    {
        fReceivedMessage = msg.toUtf8();
        onSocketReadyRead();
    }

    bool NodeWS::isRemoteRequest() const
    {
        if ( !fIsThinClient ) {
            return false; // all are considered local in this case
        }

        switch ( fActiveRequest.getType() ) {
            // remote
            case GetBlockNumber: return true;
            case GetBalance: return true;
            case GetTransactionCount: return true;
            case SendRawTransaction: return true;
            case GetGasPrice: return true;
            case EstimateGas: return true;
            case NewBlockFilter: return true;
            case NewEventFilter: return true;
            case GetFilterChanges: return true;
            case UninstallFilter: return true;
            case GetTransactionByHash: return true;
            case GetBlock: return true;
            case GetTransactionReceipt: return true;
            case Call: return true;
            // local
            case NoRequest: return false;
            case NewAccount: return false;
            case UnlockAccount: return false;
            case SignTransaction: return false;
            case GetAccountRefs: return false;
            case SendTransaction: return false;
            case GetClientVersion: return false;
            case GetNetVersion: return false;
            case GetSyncing: return false;
            case GetPeerCount: return false; // only "eth" available
            case GetLogs: return false; // we could use remote but this is a very heavy call, better not allow it
        }

        return false; // better safe than sorry
    }

    void NodeWS::connectWebsocket()
    {
        if ( fIsThinClient && fWebSocket.state() == QAbstractSocket::UnconnectedState && !fEndpoint.isEmpty() ) {
            EtherLog::logMsg("Connecting to WS endpoint: " + fEndpoint, LS_Info);
            fWebSocket.open(QUrl(fEndpoint));
        }
    }

    bool NodeWS::isThinClient() const
    {
        return fIsThinClient;
    }

}
