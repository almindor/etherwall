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
 * @date 2017
 *
 * Main entry point
 */

#include <QApplication>
#include <QTranslator>
#include <QQmlApplicationEngine>
#include <QDir>
#include <QQmlContext>
#include <QtQml/qqml.h>
#include <QIcon>
#include <QPixmap>
#include <QFile>
#include <QDebug>
#include "etherlog.h"
#include "settings.h"
#include "clipboard.h"
#include "accountmodel.h"
#include "accountproxymodel.h"
#include "transactionmodel.h"
#include "contractmodel.h"
#include "eventmodel.h"
#include "currencymodel.h"
#include "filtermodel.h"
#include "gethlog.h"
#include "helpers.h"
#include "remoteipc.h"

#include "trezor/trezor.h"
#include "platform/devicemanager.h"

using namespace Etherwall;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qmlRegisterType<AccountProxyModel>("AccountProxyModel", 0, 1, "AccountProxyModel");

    QCoreApplication::setOrganizationName("Etherdyne");
    QCoreApplication::setOrganizationDomain("etherwall.com");
    QCoreApplication::setApplicationName("Etherwall");
    QCoreApplication::setApplicationVersion("2.0.0");
    app.setWindowIcon(QIcon(QPixmap(":/images/icon")));

    QTranslator translator;
    translator.load("i18n/etherwall_" + QLocale::system().name());
    app.installTranslator(&translator);

    Settings settings;

    const QString gethPath = settings.value("geth/path", DefaultGethPath()).toString();
    const QString dataPath = settings.value("geth/datadir", DefaultDataDir).toString();

    // set defaults
    if ( !settings.contains("geth/path") ) {
        settings.setValue("geth/path", gethPath);
    }
    if ( !settings.contains("geth/datadir") ) {
        settings.setValue("geth/datadir", dataPath);
    }

    ClipboardAdapter clipboard;
    EtherLog log;
    GethLog gethLog;

    // get SSL cert for https://data.etherwall.com
    const QSslCertificate certificate(EtherWall_Cert.toUtf8());
    QSslSocket::addDefaultCaCertificate(certificate);

    Trezor::TrezorDevice trezor;
    DeviceManager deviceManager(app);
    RemoteIPC ipc(gethLog, "wss://api2.etherwall.com");
    CurrencyModel currencyModel;
    AccountModel accountModel(ipc, currencyModel, trezor);
    TransactionModel transactionModel(ipc, accountModel);
    ContractModel contractModel(ipc);
    FilterModel filterModel(ipc);
    EventModel eventModel(contractModel, filterModel);

    // main connections
    QObject::connect(&accountModel, &AccountModel::accountsReady, &deviceManager, &DeviceManager::startProbe);
    QObject::connect(&deviceManager, &DeviceManager::deviceInserted, &trezor, &Trezor::TrezorDevice::onDeviceInserted);
    QObject::connect(&deviceManager, &DeviceManager::deviceRemoved, &trezor, &Trezor::TrezorDevice::onDeviceRemoved);
    QObject::connect(&trezor, &Trezor::TrezorDevice::transactionReady, &transactionModel, &TransactionModel::onRawTransaction);

    // for QML only
    QmlHelpers qmlHelpers;

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("settings", &settings);
    engine.rootContext()->setContextProperty("ipc", &ipc);
    engine.rootContext()->setContextProperty("trezor", &trezor);
    engine.rootContext()->setContextProperty("accountModel", &accountModel);
    engine.rootContext()->setContextProperty("transactionModel", &transactionModel);
    engine.rootContext()->setContextProperty("contractModel", &contractModel);
    engine.rootContext()->setContextProperty("filterModel", &filterModel);
    engine.rootContext()->setContextProperty("eventModel", &eventModel);
    engine.rootContext()->setContextProperty("currencyModel", &currencyModel);
    engine.rootContext()->setContextProperty("clipboard", &clipboard);
    engine.rootContext()->setContextProperty("log", &log);
    engine.rootContext()->setContextProperty("geth", &gethLog);
    engine.rootContext()->setContextProperty("helpers", &qmlHelpers);

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    if ( settings.contains("program/v2firstrun") ) {
        ipc.init();
    } else {
        settings.remove("geth/testnet"); // cannot be set from before in the firstrun, not compatible with thin client
    }

    return app.exec();
}
