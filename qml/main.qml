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
import QtQuick.Window 2.0
import "components"

ApplicationWindow {
    property int dpi: Screen.pixelDensity * 25.4;

    id: appWindow
    visible: true
    minimumWidth: 8 * dpi
    minimumHeight: 6 * dpi

    width: 8 * dpi
    height: 6 * dpi

    Component.onCompleted: {
        setX(Screen.width / 2.0 - width / 2.0)
        setY(Screen.height / 2.0 - height / 2.0)
    }

    title: qsTr("Etherdyne Ethereum Wallet") + " " + Qt.application.version + ' [' + ipc.clientVersion + ']'

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
        width: 5 * dpi

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
        running: ipc.starting || ipc.busy || ipc.syncing

    }

    FirstTimeDialog {
        visible: !settings.contains("program/firstrun")
    }

    TabView {
        id: tabView
        anchors.fill: parent

        AccountsTab {}

        TransactionsTab {}

        CurrencyTab {}

        SettingsTab {}

        LogTab {}

        GethTab {}
    }

    statusBar: StatusBar {
        height: 38
        enabled: !ipc.busy

        Row {
            id: leftButtonsRow
            ToolButton {
                id: blockButton
                height: 32
                width: 32
                enabled: parent.enabled && (ipc.connectionState > 0)
                iconSource: "/images/block"
                tooltip: qsTr("Block number: ") + transactionModel.blockNumber
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
                tooltip: qsTr("Gas price: ") + transactionModel.gasPrice
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

        Text {
            anchors.centerIn: parent
            visible: !ipc.syncing
            text: ipc.closing ? qsTr("Closing app") : (ipc.starting ? qsTr("Starting Geth...") : qsTr("Ready"))
        }

        Text {
            anchors.centerIn: parent
            visible: ipc.syncing
            text: qsTr("Synchronized ") + Math.max(0, ipc.currentBlock - ipc.startingBlock) + qsTr(" out of ") + Math.max(0, ipc.highestBlock - ipc.startingBlock) + qsTr(" blocks")
        }

        ProgressBar {
            anchors.leftMargin: 0.05 * dpi
            anchors.rightMargin: 0.05 * dpi
            anchors.left: leftButtonsRow.right
            anchors.right: rightButtonsRow.left
            anchors.verticalCenter: parent.verticalCenter
            z: -999
            visible: ipc.syncing
            value: Math.max(0, ipc.currentBlock - ipc.startingBlock) / Math.max(1, ipc.highestBlock - ipc.startingBlock)
        }

        Row {
            id: rightButtonsRow
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
                enabled: !ipc.starting
                tooltip: qsTr("Connection state: ") + (ipc.connectionState > 0 ? (qsTr("connected with ", "connection state connected with X peers") + ipc.peerCount + qsTr(" peers", "connection status, peercount")) : qsTr("disconnected", "connection state"))
                onClicked: {
                    ipc.connectToServer()
                }
            }
        }
    }

}
