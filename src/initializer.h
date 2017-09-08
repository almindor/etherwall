#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <QObject>
#include <QNetworkAccessManager>

namespace Etherwall {

    class Initializer : public QObject
    {
        Q_OBJECT
    public:
        explicit Initializer(QObject *parent = nullptr);
        Q_INVOKABLE void start();
    signals:
        void initDone(const QString& version, const QString& endpoint, const QString& warning) const;
    private slots:
        void httpRequestDone(QNetworkReply *reply);
    private:
        QNetworkAccessManager fNetManager;
    };

}

#endif // INITIALIZER_H
