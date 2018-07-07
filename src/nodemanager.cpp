#include "nodemanager.h"
#include "helpers.h"
#include "etherlog.h"
#include <QSettings>
#include <QUrl>
#include <QJsonDocument>
#include <QFile>
#include <QDateTime>
#include <QStandardPaths>
#include <QProcess>

#define DOWNLOAD_BASE_PATH "https://gethstore.blob.core.windows.net/builds/geth-"

#ifdef Q_OS_WIN32
#define DOWNLOAD_OS_STR QStringLiteral("windows-386")
#define DOWNLOAD_OS_POSTFIX QStringLiteral(".zip")
#define EXTRACT_CMD "TODO"
#define PUBKEY_N_HEX "TODO"
#define PUBKEY_E_HEX "TODO"
#endif

#ifdef Q_OS_MACX
#define DOWNLOAD_OS_STR QStringLiteral("darwin-amd64")
#define DOWNLOAD_OS_POSTFIX QStringLiteral(".tar.gz")
#define EXTRACT_CMD "/usr/bin/tar"
#define PUBKEY_N_HEX "TODO"
#define PUBKEY_E_HEX "TODO"
#endif

#ifdef Q_OS_LINUX
#define DOWNLOAD_OS_STR QStringLiteral("linux-amd64")
#define DOWNLOAD_OS_POSTFIX QStringLiteral(".tar.gz")
#define EXTRACT_CMD "/usr/bin/tar"
#define PUBKEY_N_HEX "C05AB73F6E6D0B0F412CCA63E858A7F72F96EBD02348A037CDD1E1D11432AED756E54D463619C2BB05CAC4EF24E9EE2C98D82DC609722F9BFD8C4FC963A7B8A5AA195694C8846B166E8472008AFE34DB51B5866E55ECCA4329A5CBA0C71B28B764EB0B542DFC7CBBE2AF3858031005F27DEF77459B49A849E6FDE777296175DB7B3F71E5F275FE2BB85CA9C96D3881A2877A96F6DD6367CFDF2442A5B196649645EB29EE00FBACE1841344756E52B6567564B13022F8E6D425DAD197F68D618CF068FC32B12A197FE590010ADDFD6731FC54E59E9E88383BF11FCF264C851ADCA9C234EC71043DE2D9E5C859C4777EAC8C9C801AA15C4A4332F4A1D92E4C73BED9AC4D79CBDF5D4EEB37E935784035DB56C90D08F1A8A86D80A7C28B083EA6BC3DFAC192FD99C52F13FEB35B2CE73D47A534CEEB4610A6762850F49B2F844668506E024CD2838DE24B745A6972E4F2536E9E95B043163CE7DF796472A47FFCAF83DC142F043B943BC33BA9C3F8709B7FA1F900E00BB76B9000F686D5B2A4F9B8A868B0C5DB38875BA8F00D12B145AD05C8DB4FDB26FA9C2335C094AEEB299A450C6A4C57108DF1F135E15661984E6DF0BDD1EA949E550485F016CFFB5F943222A03191841F570778110AF8D5CFF812C69E3138C804B48B14833B1ABCF5B11351C3181D02281F140A1195A8D5A5D0BA26D065C6C5483654D17E813A6979EB3BAB"
#define PUBKEY_E_HEX "010001"
#endif

namespace Etherwall {

    NodeManager::NodeManager(FileDownloader& fileDownloader) : QObject(),
        fVerifier(PUBKEY_N_HEX, PUBKEY_E_HEX), fUpgrader(fVerifier, fileDownloader), fNodeName(),
        fNetManager(), fCurrentTag(), fLatestTag(),
        fCurrentVersion(0), fLatestVersion(0), fDownloadLink(),
        fRequiredTooling(checkRequiredTooling())
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

        emit nodeNameChanged(fNodeName);

        qint64 lastrun = settings.value(settingsPrefix() + "/lastrun", 0).toLongLong(&ok);
        if ( !ok ) {
            EtherLog::logMsg("Invalid lastrun in settings", LS_Error);
            throw QString("Invalid lastrun in settings");
        }

        if ( !fRequiredTooling ) {
            return; // no need to bother server if we can't extract the node
        }

        if ( QDateTime::currentSecsSinceEpoch() - lastrun > 3600 * 24 ) {
            QNetworkRequest release(QUrl("https://api.github.com/repos/ethereum/go-ethereum/releases/latest"));
            fNetManager.get(release);
        } else {
            loadResults();
        }
    }

    const QString NodeManager::getNodeName() const
    {
        return fNodeName;
    }

    const QString NodeManager::cmdLineArgs() const
    {
        return QString(); // TODO
    }

    bool NodeManager::getCanUpgrade() const
    {
        return (fCurrentVersion > 0 && fLatestVersion > 0 && fCurrentVersion <= fLatestVersion); // TODO: <
    }

    const QString NodeManager::currentTag() const
    {
        return fCurrentTag;
    }

    const QString NodeManager::latestTag() const
    {
        return fLatestTag;
    }

    void NodeManager::upgrade()
    {
        if ( fDownloadLink.isEmpty() ) {
            emit error("Attempted download before known link");
            return;
        }

        fUpgrader.upgrade(fDownloadLink, QStringList(fNodeName));
    }

    void NodeManager::onClientVersionChanged(const QString& ver)
    {
        fCurrentTag = tagFromFullVersion(ver);
        fCurrentVersion = Helpers::parseVersion(fCurrentTag);

        emit currentVersionChanged();
        emit anyVersionChanged();

        checkVersion();
    }

    void NodeManager::onHttpRequestDone(QNetworkReply* reply)
    {
        const QString uri = reply->url().fileName();

        if ( uri == "latest" ) { // both geth and parity
            return handleRelease(Helpers::parseHTTPReply(reply));
        } else if ( fNodeType == NodeTypes::Geth && uri == "tags" ) { // just geth
            return handleTags(Helpers::parseHTTPReply(reply));
        }
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

        emit latestVersionChanged();
        emit anyVersionChanged();

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
            fDownloadLink = QUrl(DOWNLOAD_BASE_PATH + DOWNLOAD_OS_STR + "-" + version + "-" + commit + DOWNLOAD_OS_POSTFIX);

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

        checkVersion();
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

        emit latestVersionChanged();
        emit anyVersionChanged();

        checkVersion();
    }

    void NodeManager::checkVersion() const
    {
        if ( fCurrentVersion == 0 || fLatestVersion == 0 ) {
            return; // not ready
        }

        if ( fCurrentVersion <= fLatestVersion ) { // TODO: <
            emit newNodeVersionAvailable(fNodeName, fCurrentTag, fLatestTag);
        }
    }

    bool NodeManager::checkRequiredTooling() const
    {
#ifdef Q_OS_WIN32
        // TODO
#else
        const QFileInfo tar(EXTRACT_CMD); // pretty std. location, works on mac and most linux/unix machines
        return (tar.exists() && tar.isExecutable());
#endif
    }
}
