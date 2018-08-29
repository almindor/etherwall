#include "nodemanager.h"
#include "helpers.h"
#include "etherlog.h"
#include <QSettings>
#include <QUrl>
#include <QJsonDocument>
#include <QFile>
#include <QDateTime>

#define DOWNLOAD_BASE_PATH "https://gethstore.blob.core.windows.net/builds/geth-"

#ifdef Q_OS_WIN32
#define DOWNLOAD_OS_STR QStringLiteral("windows-386")
#define DOWNLOAD_OS_POSTFIX QStringLiteral(".zip")
#endif

#ifdef Q_OS_MACX
#define DOWNLOAD_OS_STR QStringLiteral("darwin-amd64")
#define DOWNLOAD_OS_POSTFIX QStringLiteral(".tar.gz")
#endif
// linux does not do downloads

#ifdef Q_OS_LINUX
#define DOWNLOAD_OS_STR QStringLiteral("linux-amd64")
#define DOWNLOAD_OS_POSTFIX QStringLiteral(".tar.gz")
#endif

// linux does not do downloads

namespace Etherwall {

    NodeManager::NodeManager(QObject* parent) : QObject(parent),
        fNodeName(), fNetManager(), fLatestTag(), fDownloadLink(), fCurrentTag(),
        fLatestVersion(0), fCurrentVersion(0)
    {
        connect(&fNetManager, &QNetworkAccessManager::finished, this, &NodeManager::onHttpRequestDone);

        const QSettings settings;
        bool ok = false;
        fNodeType = (NodeTypes) settings.value("node/type", NodeTypes::Geth).toInt(&ok);
        if ( !ok ) {
            EtherLog::logMsg("Invalid node type in settings", LS_Error);
            throw QString("Invalid node type in settings");
        }

        if ( fNodeType == Parity ) {
            fNodeName = "parity";
            EtherLog::logMsg("Unsupported node type", LS_Error);
            throw QString("Unsupported node type");
        } else {
            fNodeName = "geth";
        }

        qint64 lastrun = settings.value(settingsPrefix() + "/lastrun", 0).toLongLong(&ok);
        if ( !ok ) {
            EtherLog::logMsg("Invalid lastrun in settings", LS_Error);
            throw QString("Invalid lastrun in settings");
        }

        if ( QDateTime::currentSecsSinceEpoch() - lastrun > 3600 * 24 ) {
            QNetworkRequest release(QUrl("https://api.github.com/repos/ethereum/go-ethereum/releases/latest"));
            fNetManager.get(release);
        } else {
            loadResults();
        }
    }

    const QString NodeManager::cmdLineArgs() const
    {
        return QString(); // TODO
    }

    void NodeManager::onClientVersionChanged(const QString& ver)
    {
        fCurrentTag = tagFromFullVersion(ver);
        fCurrentVersion = Helpers::parseVersion(fCurrentTag);

        checkVersions();
    }

    void NodeManager::onHttpRequestDone(QNetworkReply* reply)
    {
        const QString uri = reply->url().fileName();

        if ( uri == "latest" ) { // both geth and parity
            return handleRelease(Helpers::parseHTTPReply(reply));
        } else if ( fNodeType == NodeTypes::Geth && uri == "tags" ) { // just geth
            return handleTags(Helpers::parseHTTPReply(reply));
        }

        EtherLog::logMsg("Unknown uri from reply: " + uri, LS_Error);
        emit error("Unknown uri from reply: " + uri);
    }

    const QString NodeManager::tagFromFullVersion(const QString& ver) const
    {
        QRegExp reg("^Geth/(v[0-9]+\\.[0-9]+\\.[0-9]+).*$");
        reg.indexIn(ver);
        if ( reg.captureCount() == 1 ) try { // it's geth
            return reg.cap(1);
        } catch ( ... ) {
            return 0;
        }

        return 0;
    }

    void NodeManager::handleRelease(const QJsonDocument& reply)
    {
        const QJsonObject resObj = reply.object();
        if ( !resObj.contains("tag_name") ) { // probably a failure
            EtherLog::logMsg("Invalid reply", LS_Error);
            emit error("Invalid reply");
            return;
        }

        fLatestTag = resObj.value("tag_name").toString();
        fLatestVersion = Helpers::parseVersion(fLatestTag);

        QNetworkRequest tags(QUrl("https://api.github.com/repos/ethereum/go-ethereum/tags"));
        fNetManager.get(tags);
    }

    void NodeManager::handleTags(const QJsonDocument& reply)
    {
        const QJsonArray list = reply.array();

        foreach ( const QJsonValue& val, list ) {
            if ( !val.isObject() ) {
                continue;
            }

            const QJsonObject obj = val.toObject();
            if ( !obj.contains("commit") || !obj.contains("name") ) {
                continue;
            }

            const QString name = obj.value("name").toString();
            if ( name != fLatestTag ) {
                continue;
            }

            const QJsonValue commitVal = obj.value("commit");
            if ( !commitVal.isObject() ) {
                continue;
            }

            const QJsonObject commitObj = commitVal.toObject();
            if ( !commitObj.contains("sha") ) {
                continue;
            }

            const QString sha = commitObj.value("sha").toString();
            if ( sha.length() < 8 ) {
                emit error("Invalid commit sha: " + sha); // very odd?
                continue;
            }

            const QStringRef version = fLatestTag.midRef(1);
            const QStringRef commit = sha.leftRef(8);
            fDownloadLink = DOWNLOAD_BASE_PATH + DOWNLOAD_OS_STR + "-" + version + "-" + commit + DOWNLOAD_OS_POSTFIX;

            saveResults();
            return;
        }

        EtherLog::logMsg("Corresponding tag not found", LS_Error);
        emit error("Corresponding tag not found");
    }


    const QString NodeManager::settingsPrefix() const
    {
        switch ( fNodeType ) {
            case NodeTypes::Geth: return "/node/geth";
            case NodeTypes::Parity: return "/node/parity";
        }

        return "/node/unknown";
    }

    void NodeManager::saveResults()
    {
        QSettings settings;
        settings.beginGroup(settingsPrefix());
        settings.setValue("latest_tag", fLatestTag);
        settings.setValue("latest_version", fLatestVersion);
        settings.setValue("download_link", fDownloadLink);
        settings.setValue("lastrun", QDateTime::currentSecsSinceEpoch());

        checkVersions();
    }

    void NodeManager::loadResults()
    {
        bool ok = false;
        QSettings settings;
        settings.beginGroup(settingsPrefix());

        fLatestVersion = settings.value("latest_version", fLatestVersion).toInt(&ok);
        if ( !ok ) {
            EtherLog::logMsg("Invalid latest version in settings", LS_Error);
            throw QString("Invalid latest version in settings");
        }

        fLatestTag = settings.value("latest_tag", fLatestTag).toString();
        fDownloadLink = settings.value("download_link", fDownloadLink).toString();
        checkVersions();
    }

    void NodeManager::checkVersions() const
    {
        if ( fCurrentVersion == 0 || fLatestVersion == 0 ) {
            return; // not ready
        }

        if ( fCurrentVersion < fLatestVersion ) {
            emit newNodeVersionAvailable(fNodeName, fCurrentTag, fLatestTag);
        }
    }
}
