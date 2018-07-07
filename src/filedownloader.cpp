#include "filedownloader.h"
#include "etherlog.h"
#include <QFile>
#include <QSignalMapper>
#include <QDateTime>
#include <QFileInfo>
#include <QVector>

namespace Etherwall {

    FileDownload::FileDownload(FileDownloader& owner, QNetworkReply* reply, const QString& destination, const QUuid& groupID) :
        fOwner(owner), fReply(reply), fDestination(destination), fDestinationFile(destination),
        fLastProgressUpdate(0), fProgress(0), fAborted(false), fID(QUuid::createUuid()), fGroupID(groupID)
    {
        connect(reply, &QNetworkReply::readyRead, this, &FileDownload::onHttpReadyRead);
        connect(reply, &QNetworkReply::downloadProgress, this, &FileDownload::onDownloadProgress);

        if ( !fDestinationFile.open(QIODevice::WriteOnly) ) {
            emit fOwner.error(fDestinationFile.errorString());
        }
    }

    FileDownload::~FileDownload()
    {
        if ( isRunning() ) {
            abort();
        }

        fReply->deleteLater();
    }

    void FileDownload::abort()
    {
        if ( !fReply->isRunning() ) {
            return;
        }

        fAborted = true;
        fReply->abort();
        fOwner.onRunningChanged(fReply);
        fDestinationFile.close();
    }

    void FileDownload::complete()
    {
        fDestinationFile.close();
    }

    const QUuid FileDownload::id() const
    {
        return fID;
    }

    const QUuid FileDownload::groupId() const
    {
        return fGroupID;
    }

    bool FileDownload::isRunning() const
    {
        return fReply->isRunning();
    }

    bool FileDownload::isAborted() const
    {
        return fAborted;
    }

    double FileDownload::progress() const
    {
        return fProgress;
    }

    const QString FileDownload::destination() const
    {
        return fDestination;
    }

    const QVariant FileDownload::value(FileDownloadRoles role) const
    {
        switch ( role ) {
            case FileNameRole: return QFileInfo(fDestinationFile).fileName();
            case FilePathRole: return fDestination;
            case FileProgressRole: return fProgress;
            case FileRunningRole: return fReply->isRunning();
        }

        return "Unknown";
    }

    void FileDownload::onHttpReadyRead()
    {
        const QByteArray buf = fReply->readAll();
        if ( buf.isEmpty() && fReply->error() != QNetworkReply::NoError ) {
            emit fOwner.error(fReply->errorString());
            return;
        }

        if ( fDestinationFile.write(buf) < 0 ) {
            emit fOwner.error(fDestinationFile.errorString());
            return;
        }
    }

    void FileDownload::onDownloadProgress(qint64 bytesRead, qint64 totalBytes)
    {
        if ( QDateTime::currentSecsSinceEpoch() - fLastProgressUpdate < fOwner.getProgressInterval() ) {
            return;
        }

        if ( totalBytes == 0 ) {
            emit fOwner.error("Unexpected zero-sized total in download progress event");
            return;
        }
        fProgress = (double) bytesRead / (double) totalBytes;
        fOwner.onProgressChanged(fReply);
    }

    bool FileDownload::isReply(const QNetworkReply* reply) const
    {
        return fReply == reply;
    }

    // FILE DOWNLOADER

    FileDownloader::FileDownloader() :
        fNetManager(), fDownloads(),
        fProgressInterval(1)
    {
        connect(&fNetManager, &QNetworkAccessManager::finished, this, &FileDownloader::onHttpRequestDone);
    }

    QHash<int, QByteArray> FileDownloader::roleNames() const
    {
        QHash<int, QByteArray> roles;
        roles[FileNameRole] = "file_name";
        roles[FilePathRole] = "file_path";
        roles[FileProgressRole] = "file_progress";
        roles[FileRunningRole] = "file_running";

        return roles;
    }

    int FileDownloader::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);
        return fDownloads.size();
    }

    QVariant FileDownloader::data(const QModelIndex& index, int role) const
    {
        return fDownloads.at(index.row())->value((FileDownloadRoles) role);
    }

    int FileDownloader::getProgressInterval() const
    {
        return fProgressInterval;
    }

    const QUuid FileDownloader::download(const QUrl& source, const QString& destination, const QUuid& groupID)
    {
        foreach ( const FileDownload* download, fDownloads ) {
            if ( download->isRunning() && download->destination() == destination ) {
                EtherLog::logMsg("existing download in progress", LS_Error);
                return QUuid();
            }
        }

        QNetworkRequest download(source);
        QNetworkReply* reply = fNetManager.get(download);
        if ( reply->error() != QNetworkReply::NoError ) {
            emit error(reply->errorString());
            return QUuid();
        }

        FileDownload* dl = new FileDownload(*this, reply, destination, groupID);

        beginInsertRows(QModelIndex(), fDownloads.size(), fDownloads.size());
        fDownloads.append(dl);
        endInsertRows();

        emit downloadCountChanged(getDownloadCount());

        return dl->id();
    }

    void FileDownloader::abort(int index)
    {
        fDownloads[index]->abort();
        emit downloadCountChanged(getDownloadCount());
    }

    int FileDownloader::getDownloadCount() const
    {
        int count = 0;
        foreach ( const FileDownload* download, fDownloads ) {
            if ( download->isRunning() ) {
                count++;
            }
        }

        return count;
    }

    bool FileDownloader::isDownloadGroupComplete(const QUuid& groupID) const
    {
        if ( groupID.isNull() ) {
            return false;
        }

        bool result = false;
        foreach ( const FileDownload* download, fDownloads ) {
            if ( download->groupId() == groupID ) {
                result = true; // at least one

                if ( download->isRunning() || download->isAborted() || download->progress() < 1.0 ) {
                    return false; // any not done we're not done
                }
            }
        }

        return result;
    }

    void FileDownloader::onHttpRequestDone(QNetworkReply* reply)
    {
        for ( int i = 0; i < fDownloads.size(); i++ ) {
            if ( fDownloads.at(i)->isReply(reply) ) {
                if ( !fDownloads.at(i)->isAborted() ) {
                    fDownloads[i]->complete();
                    emit downloadComplete(fDownloads.at(i)->id());

                    const QUuid groupID = fDownloads.at(i)->groupId();
                    if ( isDownloadGroupComplete(groupID) ) {
                        emit downloadGroupComplete(groupID);
                    }
                }
                const QVector<int> roles(1, FileRunningRole);
                emit dataChanged(createIndex(i, 0), createIndex(i, 0), roles);
                return;
            }
        }

        emit error("Unable to find finished download request in list");
    }

    void FileDownloader::onProgressChanged(QNetworkReply* reply)
    {
        for ( int i = 0; i < fDownloads.size(); i++ ) {
            if ( fDownloads.at(i)->isReply(reply) ) {
                const QVector<int> roles(1, FileProgressRole);
                emit dataChanged(createIndex(i, 0), createIndex(i, 0), roles);
                return;
            }
        }
    }

    void FileDownloader::onRunningChanged(QNetworkReply* reply)
    {
        for ( int i = 0; i < fDownloads.size(); i++ ) {
            if ( fDownloads.at(i)->isReply(reply) ) {
                const QVector<int> roles(1, FileRunningRole);
                emit dataChanged(createIndex(i, 0), createIndex(i, 0), roles);
                return;
            }
        }
    }

}
