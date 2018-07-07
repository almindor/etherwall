#include "upgrader.h"
#include <QStandardPaths>
#include <QCoreApplication>
#include <QProcess>
#include <QDir>
#include <QFile>

#ifdef Q_OS_WIN32
#define EXTRACT_CMD "TODO"
#endif

#ifdef Q_OS_MACX
#define EXTRACT_CMD "/usr/bin/tar"
#endif

#ifdef Q_OS_LINUX
#define EXTRACT_CMD "/usr/bin/tar"
#endif

namespace Etherwall {

    Upgrader::Upgrader(PGP::VerifyRSA& verifier, FileDownloader& downloader) : QThread(0),
        fVerifier(verifier), fDownloader(downloader), fDownloadGroupID(), fVerifyID()
    {
        connect(&fVerifier, &PGP::VerifyRSA::error, this, &Upgrader::onVerificationError);
        connect(&fVerifier, &PGP::VerifyRSA::verified, this, &Upgrader::onVerificationComplete);
        connect(&fDownloader, &FileDownloader::error, this, &Upgrader::onDownloadError);
        connect(&fDownloader, &FileDownloader::downloadGroupComplete, this, &Upgrader::onDownloadGroupComplete);
    }

    void Upgrader::upgrade(const QUrl& source, const QStringList& fileList)
    {
        if ( !fDownloadGroupID.isNull() ) {
            emit error("Upgrade already in progress");
            return;
        }

        const QDir tmpDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
        fArchive = tmpDir.absoluteFilePath(source.fileName());
        fFileList = fileList;

        fDownloadGroupID = QUuid::createUuid();
        fDownloader.download(source, fArchive, fDownloadGroupID); // get archive
        fDownloader.download(QUrl(source.toString() + ".asc"), fArchive + ".asc", fDownloadGroupID); // get signature
    }

    void Upgrader::run() // unpack
    {
        const QDir archiveDir = extract();
        const QDir destDir = QDir(QCoreApplication::applicationDirPath());

        foreach ( const QString& filePath, fFileList ) {
            const QString sourcePath = archiveDir.absoluteFilePath(filePath);
            const QString baseName = QFileInfo(sourcePath).fileName();
            if ( !QFile::copy(sourcePath, destDir.absoluteFilePath(baseName)) ) {
                emit error("Unable to copy " + baseName);
                return;
            }
        }
    }

    void Upgrader::onDownloadGroupComplete(const QUuid& groupID)
    {
        if ( !groupID.isNull() && groupID == fDownloadGroupID ) {
            fDownloadGroupID = QUuid(); // download part done
            if ( isRunning() ) {
                emit error("Extraction already in progress");
                return;
            }

            fVerifyID = fVerifier.verify(fArchive + ".asc", fArchive);
            if ( fVerifyID.isNull() ) {
                emit error("Verification already in progress");
                return;
            }
        }
    }

    void Upgrader::onDownloadError(const QString& error)
    {
        emit Upgrader::error(error);
    }

    void Upgrader::onVerificationComplete(const QUuid& id, bool valid)
    {
        if ( id != fVerifyID ) {
            emit Upgrader::error("Verifier mismatch");
            return;
        }
        if ( !valid ) {
            emit Upgrader::error("Downloaded node verification failure");
            return;
        }

        start(); // unpack
    }

    void Upgrader::onVerificationError(const QUuid& id, const QString& error)
    {
        if ( id != fVerifyID ) {
            emit Upgrader::error("Unknown verifier run instance: " + id.toString());
        }

        emit Upgrader::error(error);
    }

    const QStringList Upgrader::extractArgs(const QString& archivePath) const
    {
#ifdef Q_OS_WIN32
        // TODO
#else
        const QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

        QStringList args;
        args.append("-xzvf");
        args.append(archivePath);
        args.append("-C");
        args.append(tmp);

        return args;
#endif
    }

    const QDir Upgrader::extract()
    {
        QProcess process;
        const QStringList args = extractArgs(fArchive);

        int code = process.execute(EXTRACT_CMD, args);
        if ( code != 0 ) {
            emit error("Error extracting archive: " + process.errorString());
        }

        const QString dirPath = QFileInfo(QFileInfo(fArchive).completeBaseName()).completeBaseName(); // remove .gz and then .tar
        return QDir(dirPath);
    }


}
