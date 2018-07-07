#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QAbstractListModel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUuid>
#include <QUrl>
#include <QList>

namespace Etherwall {

    class FileDownloader;

    enum FileDownloadRoles {
        FileNameRole = Qt::UserRole + 1,
        FilePathRole,
        FileProgressRole,
        FileRunningRole
    };

    class FileDownload : public QObject
    {
        friend class FileDownloader;
        Q_OBJECT
    public:
        FileDownload(FileDownloader& owner, QNetworkReply* reply, const QString& destination, const QUuid& groupID);
        virtual ~FileDownload() override;

        void abort();
        void complete();
        const QUuid id() const;
        const QUuid groupId() const;
        bool isRunning() const;
        bool isAborted() const;
        double progress() const;
        const QString destination() const;
        const QVariant value(FileDownloadRoles role) const;
    private slots:
        void onHttpReadyRead();
        void onDownloadProgress(qint64 bytesRead, qint64 totalBytes);
    private:
        FileDownloader& fOwner;
        QNetworkReply* fReply;
        QString fDestination;
        QFile fDestinationFile;
        qint64 fLastProgressUpdate;
        double fProgress;
        bool fAborted;
        QUuid fID;
        QUuid fGroupID;

        bool isReply(const QNetworkReply* reply) const;
    };

    class FileDownloader : public QAbstractListModel
    {
        friend class FileDownload;
        Q_OBJECT
        Q_PROPERTY(int downloadCount READ getDownloadCount() NOTIFY downloadCountChanged)
        Q_PROPERTY(int progressInterval MEMBER fProgressInterval)
    public:
        FileDownloader();

        QHash<int, QByteArray> roleNames() const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

        int getProgressInterval() const;
        const QUuid download(const QUrl& source, const QString& destination, const QUuid& groupID = QUuid());
        int getDownloadCount() const;

        Q_INVOKABLE void abort(int index);
    signals:
        void error(const QString& error) const;
        void downloadCountChanged(int count) const;
        void downloadProgress(int index, double progress) const;
        void downloadComplete(const QUuid& id) const;
        void downloadGroupComplete(const QUuid& groupID) const;
    private slots:
        void onHttpRequestDone(QNetworkReply* reply);
    private:
        QNetworkAccessManager fNetManager;
        QList<FileDownload*> fDownloads;
        int fProgressInterval;

        bool isDownloadGroupComplete(const QUuid& groupID) const;
        void onProgressChanged(QNetworkReply* reply);
        void onRunningChanged(QNetworkReply* reply);
    };

}

#endif // FILEDOWNLOADER_H
