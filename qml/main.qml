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
import QtQuick.Controls 1.1
import QtQuick.Dialogs 1.2
import "components"

ApplicationWindow {
    id: appWindow
    visible: true
    width: 800
    height: 600
    minimumHeight: 400
    minimumWidth: 650
    title: qsTr("Etherwall Ethereum Wallet")

    ErrorDialog {
        id: errorDialog
        width: 500

        Connections {
            target: ipc
            onError: {
                errorDialog.error = ipc.error
                errorDialog.open()
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        z: 10
        running: ipc.busy
    }

    TabView {
        id: tabView
        anchors.fill: parent
        enabled: !ipc.busy

        AccountsTab {}

        TransactionsTab {}

        SettingsTab {}
    }

    ConfirmDialog {
        id: connectDialog
        width: 500
        msg: "IPC already connected. Are you sure you want to reconnect?"

        onYes: {
            ipc.connectToServer(settings.value("ipc/path", "bogus"))
        }
    }

    statusBar: StatusBar {
        height: 38
        enabled: !ipc.busy

        Row {
            ToolButton {
                id: blockButton
                iconSource: "/images/block"
                tooltip: "Block number: " + transactionModel.blockNumber
                onClicked: {
                    blockField.visible = !blockField.visible

                    if ( blockField.visible ) {
                        blockField.selectAll()
                        blockField.copy()
                    }
                }
            }

            TextField {
                id: blockField
                visible: false
                width: 100
                readOnly: true
                text: transactionModel.blockNumber
            }
        }

        Row {
            anchors.right: parent.right
            ToolButton {
                iconSource: "/images/connected" + ipc.connectionState
                tooltip: "Connection state: " + ipc.connectionStateStr
                onClicked: {
                    if ( ipc.connectionState > 0 ) {
                        connectDialog.open()
                    } else {
                        ipc.connectToServer(settings.value("ipc/path", "bogus"))
                    }
                }
            }
        }
    }

}
