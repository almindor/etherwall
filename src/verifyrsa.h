#ifndef VERIFYRSA_H
#define VERIFYRSA_H

#include <QThread>
#include <QUuid>
#include <QFile>
#include <gmp.h>
#include <QCryptographicHash>
#include "signaturersa.h"

namespace PGP {

    class VerifyRSA : public QThread
    {
        Q_OBJECT
    public:
        VerifyRSA(const char* pubKeyHexN, const char* pubKeyHexE);
        const QUuid verify(const QString& sigPath, const QString dataPath);
        void run() override;
    signals:
        void verified(const QUuid& id, bool valid) const;
        void error(const QUuid& id, const QString& error) const;
    private:
        mpz_t fPubKeyN;
        mpz_t fPubKeyE;
        QString fSigPath;
        QString fDataPath;
        QUuid fID;

        bool rsaSha256Verify(const QCryptographicHash& hash, const SignatureRSA& sig);
        const QByteArray EMSA_PKCS1_v1_5(const QCryptographicHash& hash) const;
    };

}

#endif // VERIFYRSA_H
