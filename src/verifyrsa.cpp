#include "verifyrsa.h"
#include <QDebug>

namespace PGP {

    const int BUFSIZE = 1024 * 1024;

    VerifyRSA::VerifyRSA(const char* pubKeyHexN, const char* pubKeyHexE) : QThread(0)
    {
        mpz_init(fPubKeyN);
        mpz_init(fPubKeyE);
        if ( mpz_set_str(fPubKeyN, pubKeyHexN, 16) != 0 ) {
            throw QString("invalid n");
        }

        if ( mpz_set_str(fPubKeyE, pubKeyHexE, 16) != 0 ) {
            throw QString("invalid e");
        }
    }

    const QUuid VerifyRSA::verify(const QString& sigPath, const QString dataPath)
    {
        if ( isRunning() ) {
            fID = QUuid();
            emit error(fID, "Verification already in progress");
            return fID;
        }

        fSigPath = sigPath;
        fDataPath = dataPath;
        fID = QUuid::createUuid();

        start();

        return fID;
    }

    void VerifyRSA::run()
    {
        try {
            SignatureRSA sig(fSigPath);

            QFile dataFile(fDataPath);
            if ( !dataFile.open(QIODevice::ReadOnly) ) {
                throw QString("Unable to open data file");
            }

            // TODO: thread/signal/slot
            dataFile.reset();
            QCryptographicHash sha256(QCryptographicHash::Sha256);
            if ( !sha256.addData(&dataFile) ) {
                throw QString("Unable to hash file");
            }
            sha256.addData(sig.ending());

            bool result = rsaSha256Verify(sha256, sig);

            emit verified(fID, result);
        } catch ( QString err ) {
            emit error(fID, err);
        }
    }

    bool VerifyRSA::rsaSha256Verify(const QCryptographicHash& hash, const SignatureRSA& sig)
    {
        mpz_t sig_num, data, result;
        mpz_init(data);
        mpz_init(result);
        mpz_init(sig_num);

        const QByteArray digest = EMSA_PKCS1_v1_5(hash);

        mpz_import(data, digest.size(), 1, 1, 0, 0, digest.constData());
        mpz_import(sig_num, sig.rawSize(), 1, 1, 0, 0, sig.raw());

        mpz_powm_sec(result, sig_num, fPubKeyE, fPubKeyN);

        return (mpz_cmp(result, data) == 0);
    }

    const QByteArray VerifyRSA::EMSA_PKCS1_v1_5(const QCryptographicHash& hash) const {
        const quint32 keyLength = 512; // currently hardcoded since we put the key here as N/E values
        const QByteArray ASN1_DER = QByteArray::fromHex("3031300d060960864801650304020105000420"); // TODO: assign to hash object
        const QByteArray hashedData = hash.result();

        int paddingSize = keyLength - ASN1_DER.size() - (256 / 8) - 3; // keyLength - DER - hashsize - 3 metabytes
        return QByteArray::fromHex("0001") + QByteArray(paddingSize, 0xff) + QByteArray(1, 0x00) + ASN1_DER + hashedData;
    }

}
