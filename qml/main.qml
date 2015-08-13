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
/** @file main.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Main app window
 */

import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Dialogs 1.2
import "components"

ApplicationWindow {
    id: appWindow
    visible: true
    width: 800
    height: 600
    minimumHeight: 300
    minimumWidth: 500
    title: qsTr("Etherwall Ethereum Wallet")

    ErrorDialog {
        id: errorDialog
        width: Math.max(appWindow.width * 0.6, 500)
        standardButtons: StandardButton.Ok

        Connections {
            target: ipc
            onError: {
                errorDialog.error = ipc.error
                errorDialog.open()
            }
        }
    }

    TabView {
        anchors.fill: parent

        AccountsTab {}

        TransactionsTab {}

        SettingsTab {}
    }
}
