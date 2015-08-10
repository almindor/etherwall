#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include "settings.h"
#include "accountmodel.h"

using namespace Etherwall;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("NetherBoyz");
    QCoreApplication::setOrganizationDomain("etherwall.com");
    QCoreApplication::setApplicationName("Etherwall");

    Settings settings;
    AccountModel accountModel;

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("settings", &settings);
    engine.rootContext()->setContextProperty("accountModel", &accountModel);

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    return app.exec();
}
