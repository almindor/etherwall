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
import QtQml 2.2
import QtQuick.Controls 1.1
import QtQuick.Dialogs 1.0
import "components"

ApplicationWindow {
    id: appWindow
    visible: true
    width: 800
    height: 600
    minimumWidth: 800
    minimumHeight: 600
    title: qsTr("Etherdiene Ethereum Wallet") + " " + Qt.application.version

    Timer {
        id: closeTimer
        interval: 100
        running: false

        onTriggered: {
            appWindow.close()
        }
    }

    onClosing: {
        close.accepted = ipc.closeApp()

        if ( !close.accepted && !closeTimer.running ) {
            closeTimer.start()
        }
    }

    ErrorDialog {
        id: errorDialog
        width: 500

        Connections {
            target: ipc
            onError: {
                errorDialog.msg = ipc.error
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
                height: 32
                width: 32
                enabled: parent.enabled && (ipc.connectionState > 0)
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

            ToolButton {
                id: gasButton
                height: 32
                width: 32
                enabled: parent.enabled && (ipc.connectionState > 0)
                iconSource: "/images/gas"
                tooltip: "Gas price: " + transactionModel.gasPrice
                onClicked: {
                    gasField.visible = !gasField.visible

                    if ( gasField.visible ) {
                        gasField.selectAll()
                        gasField.copy()
                    }
                }
            }

            TextField {
                id: gasField
                visible: false
                width: 200
                readOnly: true
                text: transactionModel.gasPrice
            }
        }

        Row {
            anchors.right: parent.right
            ToolButton {
                function getQuality(cs, pc) {
                    if ( cs <= 0 ) {
                        return 0; // disconnected
                    }

                    if ( pc > 6 ) {
                        return 3; // high
                    } else if ( pc > 3 ) {
                        return 2; // medium
                    } else {
                        return 1; // low
                    }
                }

                iconSource: "/images/connected" + getQuality(ipc.connectionState, ipc.peerCount)
                height: 32
                width: 32
                tooltip: "Connection state: " + (ipc.connectionState > 0 ? ("connected with " + ipc.peerCount + " peers") : "disconnected")
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
