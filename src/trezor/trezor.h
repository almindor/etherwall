#ifndef TREZOR_H
#define TREZOR_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QQueue>
#include <QTimer>
#include <QVariant>
#include "proto/messages.pb.h"
#include "proto/messages-common.pb.h"
#include "proto/messages-management.pb.h"
#include "proto/messages-ethereum.pb.h"
#include "wire.h"
#include "hdpath.h"
#include "ethereum/tx.h"

using namespace hw::trezor::messages;
using namespace hw::trezor::messages::common;
using namespace hw::trezor::messages::management;
using namespace hw::trezor::messages::ethereum;

namespace Trezor {

    class TrezorWorker: public QThread
    {
        Q_OBJECT
    public:
        TrezorWorker(Wire::Device& device);
        void setRequest(const Wire::Message& request);
        const Wire::Message& getReply() const;
        const QVariant getIndex() const;
    protected:
        virtual void run();
    private:
        Wire::Device& fDevice;
        Wire::Message fRequest;
        Wire::Message fReply;
    };

    class MessageQueue: public QQueue<Wire::Message>
    {
    public:
        MessageQueue();
        void lock(int type, const QVariant& index);
        void unlock();
        void push(const Wire::Message& msg);
        bool pop(Wire::Message& popped);
        const QString toString() const;
    private:
        int fLockType;
        QVariant fIndex;
    };

    class TrezorDevice: public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString deviceID READ getDeviceID NOTIFY initialized)
        Q_PROPERTY(QString version READ getVersion NOTIFY initialized)
        Q_PROPERTY(bool present READ isPresent NOTIFY presenceChanged)
        Q_PROPERTY(bool initialized READ isInitialized NOTIFY initializedChanged)
        Q_PROPERTY(bool busy READ getBusy NOTIFY busyChanged)
    public:
        explicit TrezorDevice();
        virtual ~TrezorDevice();

        bool isPresent();
        bool isInitialized();
        void getAddress(const HDPath& hdPath);
        const QString getDeviceID() const;
        const QString getVersion() const;
        void initialize();
        Q_INVOKABLE void cancel();
        Q_INVOKABLE void submitPin(const QString& pin);
        Q_INVOKABLE void submitPassphrase(const QString& pw);
        // all values in ether
        Q_INVOKABLE void signTransaction(quint32 chaindID, const QString& hdPath, const QString& from, const QString& to,
                                         const QString& valStr, quint64 nonce,
                                         const QString& gas = QString(), const QString& gasPrice = QString(),
                                         const QString& data = QString());
    signals:
        void presenceChanged(bool present) const;
        void initialized(const QString& deviceID) const;
        void initializedChanged(bool initialized) const;
        void failure(const QString& error) const;
        void matrixRequest(int type) const;
        void buttonRequest(int code) const;
        void passphraseRequest(bool onDevice) const;
        void addressRetrieved(const QString& address, const QString& hdPath) const;
        void busyChanged(bool busy) const;
        void transactionReady(const Ethereum::Tx& tx) const;
        void deviceOutdated(const QString& minVersion, const QString& curVersion) const;
        void error(const QString& error) const;
    public slots:
        void onDeviceInserted();
        void onDeviceRemoved();
        void onDirectoryChanged(const QString& path);
        void checkPresence();
    private slots:
        void workerDone();
    private:
        Wire::Device fDevice;
        TrezorWorker fWorker;
        MessageQueue fQueue;
        QString fDeviceID;
        QString fVersion;
        bool fDevicePresent;
        Ethereum::Tx fPendingTx;

        bool getBusy() const;
        void bail(const QString& err);
        const Wire::Message serializeMessage(google::protobuf::Message& msg, MessageType, const QVariant& index);
        bool parseMessage(const Wire::Message& msg_in, google::protobuf::Message& parsed) const;
        void sendMessage(google::protobuf::Message& msg, MessageType type, QVariant index = QVariant());
        void sendNext();
        void handleResponse(const Wire::Message& msg_in);
        void handleFailure(const Wire::Message& msg_in);
        void handleMatrixRequest(const Wire::Message& msg_in);
        void handleButtonRequest(const Wire::Message& msg_in);
        void handlePassphrase(const Wire::Message& msg_in);
        void handlePassphraseStateRequest(const Wire::Message& msg_in);
        void handleFeatures(const Wire::Message& msg_in);
        void handleAddress(const Wire::Message& msg_in);
        void handleTxRequest(const Wire::Message& msg_in);
    };

}

#endif // TREZOR_H
