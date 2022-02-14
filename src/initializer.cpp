#include "initializer.h"
#include "etherlog.h"
#include "helpers.h"
#include "nodeipc.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QSettings>

namespace Etherwall {


    const QString Initializer::defaultGethPath() {
    #ifdef Q_OS_WIN32
        return QApplication::applicationDirPath() + "/geth.exe";
    #else
    #ifdef Q_OS_MACX
        return QApplication::applicationDirPath() + "/geth";
    #else
        return "/usr/bin/geth";
    #endif
    #endif
    }

    Initializer::Initializer(const QString& gethPath, const QSslConfiguration& sslConfig) :
        QObject(0), fSSLConfig(sslConfig), fGethPath(gethPath)
    {
        connect(&fNetManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(httpRequestDone(QNetworkReply*)));

        replaceDeprecatedSettings();
    }

    void Initializer::replaceDeprecatedSettings() const {
        QSettings settings;
        QString argStr = settings.value("geth/args", NodeIPC::sDefaultGethArgs).toString();

        // check deprecated options and replace them
        if ( argStr.contains("--syncmode=fast") || argStr.contains("--light") || argStr.contains("--fast") ) {
            argStr = argStr.replace("--light", "--syncmode=light");
            argStr = argStr.replace("--fast", "--syncmode=snap");
            argStr = argStr.replace("--syncmode=fast", "--syncmode=snap");
            settings.setValue("geth/args", argStr);
            qDebug() << "replaced args\n";
        }
    }

    void Initializer::start()
    {
        QNetworkRequest request(QUrl("https://api.etherwall.com/api/init"));
        request.setSslConfiguration(fSSLConfig);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QJsonObject objectJson;
        const QByteArray data = QJsonDocument(objectJson).toJson();

        EtherLog::logMsg("HTTP Post request: " + data, LS_Debug);
        EtherLog::logMsg("Connecting to main Etherwall server", LS_Info);

        fNetManager.post(request, data);
    }

    void Initializer::proceed()
    {
        emit initDone(fGethPath, fVersion, fEndpoint, fWarning);
    }

    void Initializer::httpRequestDone(QNetworkReply *reply)
    {
        QSettings settings;
        QString err;
        const auto parsed = Helpers::parseHTTPReply(reply, err);
        if ( !err.isEmpty() ){
            emit error(err);
            return;
        }

        QJsonObject resObj = parsed.object();
        const bool success = resObj.value("success").toBool(false);

        if ( !success ) {
            const QString error = resObj.value("error").toString("unknown error");
            EtherLog::logMsg("Response error: " + error, LS_Error);
            emit initDone(fGethPath, "0.0.0", QString(), "Unable to connect to Etherwall server");
            return;
        }

        const QString customRemoteURL = settings.value("geth/custom", false).toBool() ? settings.value("geth/remoteURL", "").toString() : "";

        fVersion = resObj.value("version").toString("0.0.0");
        fEndpoint = customRemoteURL.isEmpty() ? resObj.value("endpoint").toString() : customRemoteURL;
        fWarning = resObj.value("warning").toString();

        if ( fWarning.isEmpty() ) {
            proceed();
        } else {
            emit warning(fVersion, fEndpoint, fWarning);
        }
    }

}
