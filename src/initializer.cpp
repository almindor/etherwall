#include "initializer.h"
#include "etherlog.h"
#include "helpers.h"
#include <QJsonDocument>
#include <QJsonObject>

namespace Etherwall {

    Initializer::Initializer(QObject *parent) : QObject(parent)
    {
        connect(&fNetManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(httpRequestDone(QNetworkReply*)));
    }

    void Initializer::start()
    {
        QNetworkRequest request(QUrl("https://data.etherwall.com/api/init"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QJsonObject objectJson;
        const QByteArray data = QJsonDocument(objectJson).toJson();

        EtherLog::logMsg("HTTP Post request: " + data, LS_Debug);
        EtherLog::logMsg("Connecting to main Etherwall server", LS_Info);

        fNetManager.post(request, data);
    }

    void Initializer::httpRequestDone(QNetworkReply *reply)
    {
        QJsonObject resObj = Helpers::parseHTTPReply(reply);
        const bool success = resObj.value("success").toBool(false);

        if ( !success ) {
            const QString error = resObj.value("error").toString("unknown error");
            EtherLog::logMsg("Response error: " + error, LS_Error);
            emit initDone("0.0.0", QString(), "Unable to connect to Etherwall server");
            return;
        }

        const QString version = resObj.value("version").toString("0.0.0");
        const QString endpoint = resObj.value("endpoint").toString();
        const QString warning = resObj.value("warning").toString();

        emit initDone(version, endpoint, warning);
    }

}
