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
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QPixmap>
#include <QDebug>
#include "etherlog.h"
#include "settings.h"
#include "clipboard.h"
#include "accountmodel.h"
#include "transactionmodel.h"

using namespace Etherwall;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Etherdiene");
    QCoreApplication::setOrganizationDomain("etherwall.com");
    QCoreApplication::setApplicationName("Etherwall");
    QCoreApplication::setApplicationVersion("0.9.1");
    app.setWindowIcon(QIcon(QPixmap(":/images/icon")));

    Settings settings;
    ClipboardAdapter clipboard;
    EtherLog log;
    EtherIPC ipc;

    const QString ipcPath = settings.value("ipc/path", DefaultIPCPath).toString();

    if ( !settings.contains("ipc/path") ) {
        settings.setValue("ipc/path", ipcPath);
    }

    AccountModel accountModel(ipc);
    TransactionModel transactionModel(ipc, accountModel);

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("settings", &settings);
    engine.rootContext()->setContextProperty("ipc", &ipc);
    engine.rootContext()->setContextProperty("accountModel", &accountModel);
    engine.rootContext()->setContextProperty("transactionModel", &transactionModel);
    engine.rootContext()->setContextProperty("clipboard", &clipboard);
    engine.rootContext()->setContextProperty("log", &log);


    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    log.log("Etherwall started");

    ipc.connectToServer(ipcPath);

    return app.exec();
}
