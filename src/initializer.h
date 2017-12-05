#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <QObject>
#include <QNetworkAccessManager>

namespace Etherwall {

    class Initializer : public QObject
    {
        Q_OBJECT
    public:
        explicit Initializer(const QString& gethPath);
        Q_INVOKABLE void start();
        Q_INVOKABLE void proceed();
        static const QString defaultGethPath();
    signals:
        void initDone(const QString& gethPath, const QString& version, const QString& endpoint, const QString& warning) const;
        void warning(const QString& version, const QString& endpoint, const QString& warning) const;
    private slots:
        void httpRequestDone(QNetworkReply *reply);
    private:
        QNetworkAccessManager fNetManager;
        QString fGethPath;
        QString fVersion;
        QString fEndpoint;
        QString fWarning;
    };

}

#endif // INITIALIZER_H
