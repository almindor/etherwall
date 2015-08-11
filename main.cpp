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
