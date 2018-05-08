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
#include "etherlogapp.h"
#include "gethlogapp.h"
#include "settings.h"
#include "clipboard.h"
#include "initializer.h"
#include "accountmodel.h"
#include "accountproxymodel.h"
#include "transactionmodel.h"
#include "contractmodel.h"
#include "eventmodel.h"
#include "currencymodel.h"
#include "filtermodel.h"
#include "tokenmodel.h"
#include "helpers.h"
#include "nodews.h"
#include "trezor/trezor.h"
#include "platform/devicemanager.h"
#include "cert.h"

using namespace Etherwall;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qmlRegisterType<AccountProxyModel>("AccountProxyModel", 0, 1, "AccountProxyModel");

    QCoreApplication::setOrganizationName("Etherdyne");
    QCoreApplication::setOrganizationDomain("etherwall.com");
    QCoreApplication::setApplicationName("Etherwall");
    QCoreApplication::setApplicationVersion("2.2.2");
    app.setWindowIcon(QIcon(QPixmap(":/images/icon")));

    QTranslator translator;
    translator.load("i18n/etherwall_" + QLocale::system().name());
    app.installTranslator(&translator);

    Settings settings;

    const QString gethPath = settings.value("geth/path", Initializer::defaultGethPath()).toString();
    const QString dataPath = settings.value("geth/datadir", NodeIPC::sDefaultDataDir).toString();

    // set defaults
    if ( !settings.contains("geth/path") ) {
        settings.setValue("geth/path", gethPath);
    }
    if ( !settings.contains("geth/datadir") ) {
        settings.setValue("geth/datadir", dataPath);
    }

    ClipboardAdapter clipboard;
    EtherLogApp log;
    GethLogApp gethLog;

    // get SSL cert for https://data.etherwall.com
    const QSslCertificate certificate(EtherWall_Cert.toUtf8());
    QSslSocket::addDefaultCaCertificate(certificate);

    Initializer initializer(gethPath);
    Trezor::TrezorDevice trezor;
    DeviceManager deviceManager(app);
    NodeWS ipc(gethLog);
    CurrencyModel currencyModel;
    AccountModel accountModel(ipc, currencyModel, trezor);
    TransactionModel transactionModel(ipc, accountModel);
    ContractModel contractModel(ipc, accountModel);
    FilterModel filterModel(ipc);
    EventModel eventModel(contractModel, filterModel);

    TokenModel tokenModel(&contractModel);

    // main connections
    QObject::connect(&initializer, &Initializer::initDone, &ipc, &NodeWS::start);
    QObject::connect(&accountModel, &AccountModel::accountsReady, &deviceManager, &DeviceManager::startProbe);
    QObject::connect(&contractModel, &ContractModel::tokenBalanceDone, &accountModel, &AccountModel::onTokenBalanceDone);
    QObject::connect(&transactionModel, &TransactionModel::confirmedTransaction, &contractModel, &ContractModel::onConfirmedTransaction);
    QObject::connect(&accountModel, &AccountModel::accountsReady, &filterModel, &FilterModel::reload);
    QObject::connect(&tokenModel, &TokenModel::selectedTokenContract, &contractModel, &ContractModel::onSelectedTokenContract);
    QObject::connect(&deviceManager, &DeviceManager::deviceInserted, &trezor, &Trezor::TrezorDevice::onDeviceInserted);
    QObject::connect(&deviceManager, &DeviceManager::deviceRemoved, &trezor, &Trezor::TrezorDevice::onDeviceRemoved);
    QObject::connect(&trezor, &Trezor::TrezorDevice::transactionReady, &transactionModel, &TransactionModel::onRawTransaction);

    // for QML only
    QmlHelpers qmlHelpers;

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("settings", &settings);
    engine.rootContext()->setContextProperty("initializer", &initializer);
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

    engine.rootContext()->setContextProperty("tokenModel", &tokenModel);

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    if ( settings.contains("program/v2firstrun") ) {
        initializer.start();
    }

    return app.exec();
}
