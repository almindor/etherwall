#ifndef UPGRADER_H
#define UPGRADER_H

#include <QThread>
#include <QUuid>
#include <QStringList>
#include "verifyrsa.h"
#include "filedownloader.h"

namespace Etherwall {

    class Upgrader : public QThread
    {
        Q_OBJECT
    public:
        Upgrader(PGP::VerifyRSA& verifier, FileDownloader& downloader);

        void upgrade(const QUrl& source, const QStringList& fileList);
        void run() override;
    signals:
        void error(const QString& error) const;
    private slots:
        void onDownloadGroupComplete(const QUuid& groupID);
        void onDownloadError(const QString& error);
        void onVerificationComplete(const QUuid& id, bool valid);
        void onVerificationError(const QUuid& id, const QString& error);
    private:
        PGP::VerifyRSA& fVerifier;
        FileDownloader& fDownloader;
        QUuid fDownloadGroupID;
        QUuid fVerifyID;
        QString fArchive;
        QStringList fFileList;

        const QStringList extractArgs(const QString& archivePath) const;
        const QDir extract();
    };

}

#endif // UPGRADER_H
