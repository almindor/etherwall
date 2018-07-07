#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include "verifyrsa.h"
#include "filedownloader.h"
#include "upgrader.h"

namespace Etherwall {

    enum NodeTypes {
        Geth = 0,
        Parity = 1
    };

    class NodeManager : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(const QString nodeName READ getNodeName NOTIFY nodeNameChanged FINAL)
        Q_PROPERTY(const QString currentVersion READ currentTag NOTIFY currentVersionChanged FINAL)
        Q_PROPERTY(const QString latestVersion READ latestTag NOTIFY latestVersionChanged FINAL)
        Q_PROPERTY(bool canUpgrade READ getCanUpgrade() NOTIFY anyVersionChanged FINAL)
    public:
        explicit NodeManager(FileDownloader& fileDownloader);

        const QString getNodeName() const;
        const QString cmdLineArgs() const;
        bool getCanUpgrade() const;
        const QString currentTag() const;
        const QString latestTag() const;

        Q_INVOKABLE void upgrade();
    public slots:
        void onClientVersionChanged(const QString& ver);
    signals:
        void error(const QString& error) const;
        void newNodeVersionAvailable(const QString& nodeName, const QString& curVersion, const QString& newVersion) const;
        void newNodeVersionReady(const QString& nodeName, const QString& curVersion, const QString& newVersion) const;
        void nodeNameChanged(const QString& nodeName) const;
        void currentVersionChanged() const;
        void latestVersionChanged() const;
        void anyVersionChanged() const;
    protected:
        void handleRelease(const QJsonDocument& reply);
        void handleTags(const QJsonDocument& reply);
    private slots:
        void onHttpRequestDone(QNetworkReply* reply);
    private:
        PGP::VerifyRSA fVerifier;
        Upgrader fUpgrader;
        NodeTypes fNodeType;
        QString fNodeName;
        QNetworkAccessManager fNetManager;
        QString fCurrentTag;
        QString fLatestTag;
        int fCurrentVersion;
        int fLatestVersion;
        QUrl fDownloadLink;
        bool fRequiredTooling;

        const QString tagFromFullVersion(const QString& ver) const;
        const QString settingsPrefix() const;
        void saveResults();
        void loadResults();
        void checkVersion() const;
        bool checkRequiredTooling() const;
    };

}

#endif // NODEMANAGER_H
