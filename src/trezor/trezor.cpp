#include "trezor.h"
#include "helpers.h"
#include "ethereum/tx.h"
#include <QDebug>
#include <QByteArray>

namespace Trezor {

    TrezorDevice::TrezorDevice() : QObject(0),
        fDevice(), fWorker(fDevice), fQueue(), fDeviceID(), fDevicePresent(false)
    {
        connect(&fWorker, &TrezorWorker::finished, this, &TrezorDevice::workerDone);
    }

    TrezorDevice::~TrezorDevice()
    {
    }

    void TrezorDevice::checkPresence()
    {
        // don't check while busy
        if ( getBusy() ) {
            return;
        }

        bool wasPresent = fDevicePresent;
        fDevicePresent = fDevice.isPresent();

        if ( wasPresent != fDevicePresent ) {
            emit presenceChanged(fDevicePresent);

            // we if inserted
            if ( fDevicePresent ) {
                initialize();
            } else { // if we removed
                fDeviceID = QString();
                fDevice.close();
                emit initializedChanged(false);
            }
        }
    }

    bool TrezorDevice::isPresent()
    {
        return fDevicePresent;
    }

    bool TrezorDevice::isInitialized()
    {
        return !fDeviceID.isEmpty();
    }

    void TrezorDevice::initialize()
    {
        if ( !isPresent() ) {
            return;
        }

        try {
            fDevice.init();
        } catch ( Trezor::Wire::Device::wire_error err ) {
            return bail("Error opening TREZOR device: " + QString(err.what()));
        }

        TrezorProtobuf::Initialize request;
        sendMessage(request, TrezorProtobuf::MessageType_Initialize);
    }

    void TrezorDevice::onDeviceInserted()
    {
        checkPresence();
    }

    void TrezorDevice::onDeviceRemoved()
    {
        checkPresence();
    }

    void TrezorDevice::onDirectoryChanged(const QString &path)
    {
        Q_UNUSED(path);
        initialize();
    }

    void TrezorDevice::getAddress(const HDPath& hdPath)
    {
        if ( !isPresent() ) {
            bail("getAddress called when trezor not present");
            return;
        }

        if ( !hdPath.valid() ) {
            bail("hd path invalid");
            return;
        }

        TrezorProtobuf::EthereumGetAddress request;
        request.set_show_display(false);

        quint32 segment;
        int index = 0;
        while ( hdPath.getSegment(index++, segment) ) {
            request.add_address_n(segment);
        }

        sendMessage(request, TrezorProtobuf::MessageType_EthereumGetAddress, hdPath.toString());
    }

    const QString TrezorDevice::getDeviceID() const
    {
        return fDeviceID;
    }

    void TrezorDevice::submitPin(const QString &pin)
    {
        TrezorProtobuf::PinMatrixAck request;
        request.set_pin(pin.toUtf8().data());
        sendMessage(request, TrezorProtobuf::MessageType_PinMatrixAck);
    }

    void TrezorDevice::submitPassphrase(const QString &pw)
    {
        TrezorProtobuf::PassphraseAck request;
        request.set_passphrase(pw.toStdString());
        sendMessage(request, TrezorProtobuf::MessageType_PassphraseAck);
    }

    void TrezorDevice::signTransaction(int chaindID, const QString& hdPath, const QString& from, const QString &to, const QString &valStr,
                                       quint64 nonce, const QString &gas, const QString &gasPrice, const QString &data)
    {
        fPendingTx.init(from, to, valStr, nonce, gas, gasPrice, data);

        TrezorProtobuf::EthereumSignTx request;

        HDPath path(hdPath);
        quint32 segment;
        int index = 0;
        while ( path.getSegment(index++, segment) ) {
            request.add_address_n(segment);
        }

        request.set_chain_id(chaindID);
        request.set_to(fPendingTx.toBytes());
        if ( fPendingTx.hasValue() ) {
            request.set_value(fPendingTx.valueBytes());
        }
        if ( nonce > 0 ) { // seems like a protobuf/trezor bug
            request.set_nonce(fPendingTx.nonceBytes());
        }
        request.set_gas_limit(fPendingTx.gasBytes());
        request.set_gas_price(fPendingTx.gasPriceBytes());

        if ( fPendingTx.dataByteSize() > 0 ) {
            request.set_data_length(fPendingTx.dataByteSize());
            request.set_data_initial_chunk(fPendingTx.dataNext(1024));
        }

        sendMessage(request, TrezorProtobuf::MessageType_EthereumSignTx);
    }

    void TrezorDevice::workerDone()
    {
        handleResponse(fWorker.getReply()); // handle first so we can have "inserts" for the meta workflows
        sendNext();
        emit busyChanged(getBusy());
    }

    bool TrezorDevice::getBusy() const
    {
        return fWorker.isRunning() || !fQueue.empty();
    }

    void TrezorDevice::cancel()
    {
        TrezorProtobuf::Cancel request;
        fQueue.unlock(); // ensure we don't try and wait for user response
        sendMessage(request, TrezorProtobuf::MessageType_Cancel);
    }

    void TrezorDevice::bail(const QString& err)
    {
        if ( fWorker.isRunning() ) {
            fWorker.terminate();
        }

        if ( !fQueue.empty() ) {
            fQueue.clear();
        }

        emit error(err);
    }

    const Wire::Message TrezorDevice::serializeMessage(google::protobuf::Message &msg, TrezorProtobuf::MessageType type, const QVariant& index)
    {
        Wire::Message msg_wire;
        msg_wire.id = type;
        msg_wire.index = index;
        msg_wire.data = std::vector<uint8_t>(msg.ByteSize());
        if ( !msg.SerializeToArray(msg_wire.data.data(), msg.ByteSize()) ) {
            bail("Could not serialize getAddress msg");
            return msg_wire;
        }

        return msg_wire;
    }

    bool TrezorDevice::parseMessage(const Wire::Message &msg_in, google::protobuf::Message& parsed) const
    {
        return parsed.ParseFromArray(msg_in.data.data(), msg_in.data.size());
    }

    void TrezorDevice::sendMessage(google::protobuf::Message& msg, TrezorProtobuf::MessageType type, const QVariant index)
    {
        const Wire::Message wireMsg = serializeMessage(msg, type, index);

        if ( type == TrezorProtobuf::MessageType_ButtonAck ||
             type == TrezorProtobuf::MessageType_EthereumTxAck ||
             type == TrezorProtobuf::MessageType_Cancel ) { // these msgs need to always go right after, no matter what we have queued already
            fQueue.prepend(wireMsg); // no need to check for lock here
        } else {
            fQueue.push(wireMsg);
        }

        sendNext();
    }

    void TrezorDevice::sendNext()
    {
        if ( !fDevice.isInitialized() ) {
            return;
        }

        if ( fWorker.isRunning() ) {
            return;
        }

        Wire::Message request;

        if ( !fQueue.pop(request)) {
            return;
        }

        fWorker.setRequest(request);
        fWorker.start();
        emit busyChanged(getBusy());
    }

    void TrezorDevice::handleResponse(const Wire::Message &msg_in)
    {
        switch ( msg_in.id ) {
            case TrezorProtobuf::MessageType_Failure: handleFailure(msg_in); return;
            case TrezorProtobuf::MessageType_PinMatrixRequest: handleMatrixRequest(msg_in); return;
            case TrezorProtobuf::MessageType_ButtonRequest: handleButtonRequest(msg_in); return;
            case TrezorProtobuf::MessageType_PassphraseRequest: handlePassphrase(msg_in); return;
            case TrezorProtobuf::MessageType_Features: handleFeatures(msg_in); return;
            case TrezorProtobuf::MessageType_EthereumAddress: handleAddress(msg_in); return;
            case TrezorProtobuf::MessageType_EthereumTxRequest: handleTxRequest(msg_in); return;
        }

        bail("Unknown msg response: " + QString::number(msg_in.id));
    }

    void TrezorDevice::handleFailure(const Wire::Message &msg_in)
    {
        if ( msg_in.id != TrezorProtobuf::MessageType_Failure ) {
            bail("Unexpected failure response: " + QString::number(msg_in.id));
            return;
        }

        TrezorProtobuf::Failure response;
        if ( !parseMessage(msg_in, response) ) {
            bail("error parsing failure response");
            return;
        }

        const QString error = QString::fromStdString(response.message());
        emit failure(error);
        fQueue.clear(); // we cannot continue after failure!
    }

    void TrezorDevice::handleMatrixRequest(const Wire::Message &msg_in)
    {
        if ( msg_in.id != TrezorProtobuf::MessageType_PinMatrixRequest ) {
            bail("Unexpected pin matrix response: " + QString::number(msg_in.id));
            return;
        }

        TrezorProtobuf::PinMatrixRequest response;
        if ( !parseMessage(msg_in, response) ) {
            bail("error parsing matrix response");
            return;
        }

        emit matrixRequest(response.type());
        fQueue.lock(TrezorProtobuf::MessageType_PinMatrixAck, fWorker.getIndex()); // we need to wait for this call before making others, saving the index
    }

    void TrezorDevice::handleButtonRequest(const Wire::Message &msg_in)
    {
        if ( msg_in.id != TrezorProtobuf::MessageType_ButtonRequest ) {
            bail("Unexpected button response: " + QString::number(msg_in.id));
            return;
        }

        TrezorProtobuf::ButtonRequest response;
        if ( !parseMessage(msg_in, response) ) {
            bail("error parsing button response");
            return;
        }
        emit buttonRequest(response.code());
        // we have to ack right away, worker will wait for actual reply
        TrezorProtobuf::ButtonAck request;
        sendMessage(request, TrezorProtobuf::MessageType_ButtonAck);
    }

    void TrezorDevice::handlePassphrase(const Wire::Message &msg_in)
    {
        if ( msg_in.id != TrezorProtobuf::MessageType_PassphraseRequest ) {
            bail("Unexpected pin matrix response: " + QString::number(msg_in.id));
            return;
        }

        TrezorProtobuf::PassphraseRequest response;
        if ( !parseMessage(msg_in, response) ) {
            bail("error parsing passphrase response");
            return;
        }

        emit passphraseRequest();
        fQueue.lock(TrezorProtobuf::MessageType_PassphraseAck, fWorker.getIndex()); // we need to wait for this call before making others, saving the index

    }

    void TrezorDevice::handleFeatures(const Wire::Message &msg_in)
    {
        if ( msg_in.id != TrezorProtobuf::MessageType_Features ) {
            bail("Unexpected init response: " + QString::number(msg_in.id));
            return;
        }

        TrezorProtobuf::Features response;
        if ( !parseMessage(msg_in, response) ) {
            bail("error parsing features response");
            return;
        }

        fDeviceID = QString::fromStdString(response.device_id());
        emit initialized(fDeviceID);
        emit initializedChanged(true);
    }

    void TrezorDevice::handleAddress(const Wire::Message &msg_in)
    {
        if ( msg_in.id != TrezorProtobuf::MessageType_EthereumAddress ) {
            bail("Unexpected get address response: " + QString::number(msg_in.id));
            return;
        }

        if ( fWorker.getIndex() < 0 ) {
            bail("Address index lost on reply");
            return;
        }

        TrezorProtobuf::EthereumAddress response;
        if ( !parseMessage(msg_in, response) ) {
            bail("error parsing address response");
            return;
        }

        const QString addressHex = Etherwall::Helpers::hexPrefix(QByteArray::fromStdString(response.address()).toHex());
        const QString hdPath = fWorker.getIndex().toString();
        emit addressRetrieved(addressHex, hdPath);
    }

    void TrezorDevice::handleTxRequest(const Wire::Message &msg_in)
    {
        if ( msg_in.id != TrezorProtobuf::MessageType_EthereumTxRequest ) {
            bail("Unexpected transaction response: " + QString::number(msg_in.id));
            return;
        }

        TrezorProtobuf::EthereumTxRequest response;
        if ( !parseMessage(msg_in, response) ) {
            bail("error parsing txsign response");
            return;
        }

        if ( response.data_length() > 0 ) {
            if ( response.data_length() > fPendingTx.dataByteSize() ) {
                bail("TREZOR requested more bytes than are in the pending tx data");
                return;
            }

            TrezorProtobuf::EthereumTxAck request;
            request.set_data_chunk(fPendingTx.dataNext(response.data_length()));
            sendMessage(request, TrezorProtobuf::MessageType_EthereumTxAck, fWorker.getIndex());
            return;
        }

        quint32 v = response.signature_v();
        std::string r = response.signature_r();
        std::string s = response.signature_s();

        fPendingTx.sign(v, r, s);
        emit transactionReady(fPendingTx);
    }

    // TrezorWorker

    TrezorWorker::TrezorWorker(Wire::Device &device): QThread(0),
        fDevice(device)
    {

    }

    void TrezorWorker::setRequest(const Wire::Message &request)
    {
        fRequest = request;
    }

    const Wire::Message &TrezorWorker::getReply() const
    {
        return fReply;
    }

    const QVariant TrezorWorker::getIndex() const
    {
        return fRequest.index;
    }

    // TrezorWorker

    void TrezorWorker::run()
    {
        fRequest.write_to(fDevice);
        fReply.read_from(fDevice);
    }

    // MessageQueue

    MessageQueue::MessageQueue() :
        QQueue(),
        fLockType(-1), fIndex(-1)
    {

    }

    void MessageQueue::lock(int type, const QVariant& index)
    {
        fLockType = type;
        fIndex = index;
    }

    void MessageQueue::unlock()
    {
        fLockType = -1;
    }

    void MessageQueue::push(const Wire::Message &msg)
    {
        if ( msg.id == fLockType ) {
            prepend(msg);
            return;
        }

        enqueue(msg);
    }

    bool MessageQueue::pop(Wire::Message& popped)
    {
        if ( empty() ) {
            return false;
        }

        if ( fLockType >= 0 && head().id != fLockType ) {
            return false;
        }

        popped = dequeue();

        if ( popped.id == fLockType ) {
            popped.index = fIndex;
            fIndex = QVariant(); // "forget"
            unlock();
        }
        return true;
    }

    const QString MessageQueue::toString() const
    {
        QString result;
        foreach ( const Wire::Message& msg, *this ) {
            result += QString::number(msg.id) + ",";
        }
        if ( result.endsWith(',') ) {
            result.remove(result.length() - 1, 1);
        }

        return result;
    }

}
