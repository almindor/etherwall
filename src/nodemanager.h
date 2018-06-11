#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>

namespace Etherwall {

    enum NodeTypes {
        Geth = 0,
        Parity = 1
    };

    class NodeManager : public QObject
    {
        Q_OBJECT
    public:
        explicit NodeManager(QObject *parent = nullptr);

        const QString cmdLineArgs() const;
    public slots:
        void onClientVersionChanged(const QString& ver);
    signals:
        void error(const QString& error) const;
        void newNodeVersionAvailable(const QString& nodeName, const QString& curVersion, const QString& newVersion) const;
    protected:
        void handleRelease(const QJsonDocument& reply);
        void handleTags(const QJsonDocument& reply);
    private slots:
        void onHttpRequestDone(QNetworkReply* reply);
    private:
        NodeTypes fNodeType;
        QString fNodeName;
        QNetworkAccessManager fNetManager;
        QString fLatestTag;
        QString fDownloadLink;
        QString fCurrentTag;
        int fLatestVersion;
        int fCurrentVersion;

        const QString tagFromFullVersion(const QString& ver) const;
        const QString settingsPrefix() const;
        void saveResults();
        void loadResults();
        void checkVersions() const;
    };

}

#endif // NODEMANAGER_H
