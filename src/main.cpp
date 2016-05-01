/*
    This file is part of etherwall.
    etherwall is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    etherwall is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with etherwall. If not, see <http://www.gnu.org/licenses/>.
*/
/** @file main.cpp
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Main entry point
 */

#include <QApplication>
#include <QTranslator>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQml/qqml.h>
#include <QIcon>
#include <QPixmap>
#include <QDebug>
#include "etherlog.h"
#include "settings.h"
#include "clipboard.h"
#include "accountmodel.h"
#include "accountproxymodel.h"
#include "transactionmodel.h"
#include "currencymodel.h"
#include "gethlog.h"

using namespace Etherwall;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qmlRegisterType<AccountProxyModel>("AccountProxyModel", 0, 1, "AccountProxyModel");

    QCoreApplication::setOrganizationName("Etherdyne");
    QCoreApplication::setOrganizationDomain("etherwall.com");
    QCoreApplication::setApplicationName("Etherwall");
    QCoreApplication::setApplicationVersion("1.0.3");
    app.setWindowIcon(QIcon(QPixmap(":/images/icon")));

    QTranslator translator;
    translator.load("i18n/etherwall_" + QLocale::system().name());
    app.installTranslator(&translator);

    Settings settings;

    const QString gethPath = settings.value("geth/path", DefaultGethPath).toString();
    const QString dataPath = settings.value("geth/datadir", DefaultDataDir).toString();
#ifdef Q_OS_WIN32
    const QString ipcPath = settings.value("ipc/path", DefaultIPCPath).toString();
#else
    const QString ipcPath = dataPath + "/geth.ipc";
#endif


    // set defaults
    if ( !settings.contains("ipc/path") ) {
        settings.setValue("ipc/path", ipcPath);
    }
    if ( !settings.contains("geth/path") ) {
        settings.setValue("geth/path", gethPath);
    }
    if ( !settings.contains("geth/datadir") ) {
        settings.setValue("geth/datadir", dataPath);
    }

    ClipboardAdapter clipboard;
    EtherLog log;
    GethLog gethLog;
    EtherIPC ipc(ipcPath, gethLog);

    CurrencyModel currencyModel;
    AccountModel accountModel(ipc, currencyModel);
    TransactionModel transactionModel(ipc, accountModel);

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("settings", &settings);
    engine.rootContext()->setContextProperty("ipc", &ipc);
    engine.rootContext()->setContextProperty("accountModel", &accountModel);
    engine.rootContext()->setContextProperty("transactionModel", &transactionModel);
    engine.rootContext()->setContextProperty("currencyModel", &currencyModel);
    engine.rootContext()->setContextProperty("clipboard", &clipboard);
    engine.rootContext()->setContextProperty("log", &log);
    engine.rootContext()->setContextProperty("geth", &gethLog);

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    if ( settings.contains("program/firstrun") ) {
        ipc.init();
    }

    return app.exec();
}
