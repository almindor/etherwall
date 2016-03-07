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
/** @file SettingsTab.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Settings tab
 */

import QtQuick 2.0
import QtQuick.Controls 1.1

Tab {
    id: accountsTab
    title: qsTr("Settings")

    Column {
        id: col
        anchors.margins: 0.2 * dpi
        anchors.fill: parent
        spacing: 0.1 * dpi

        Row {
            id: rowIPCPath
            width: parent.width

            Label {
                id: ipcPathLabel
                text: "IPC path: "
            }

            TextField {
                id: ipcPathField
                width: parent.width - ipcSaveButton.width - ipcPathLabel.width
                text: settings.value("ipc/path", "")
            }

            /*Button {
                id: ipcPathButton
                text: qsTr("Choose")

                onClicked: {
                    fileDialog.folder = settings.value("/ipc/path")
                    fileDialog.open()
                }
            }*/

            Button {
                id: ipcSaveButton
                text: qsTr("Set")

                onClicked: {
                    settings.setValue("/ipc/path", ipcPathField.text)
                }
            }

            /*FileDialog {
                id: fileDialog
                title: qsTr("IPC")
                nameFilters: ["Unix Socket IPC (*.ipc)"]

                onAccepted: {
                    ipcPathField.text = fileDialog.fileUrl
                }
            }*/
        }

        /*
        Row {
            id: rowArgs
            width: parent.width

            Label {
                id: argsLabel
                text: "Geth args: "
            }

            TextField {
                id: argsField
                width: parent.width - argsButton.width - argsLabel.width
                text: settings.value("/geth/args", "")
            }

            Button {
                id: argsButton
                text: qsTr("Set")

                onClicked: {
                    settings.setValue("/geth/args", argsField.text)
                }
            }
        }*/

        Row {
            width: parent.width

            Label {
                text: qsTr("Account unlock duration (s): ")
            }

            SpinBox {
                id: unlockDurSpinBox
                width: 100
                minimumValue: 10
                maximumValue: 3600

                value: settings.value("/ipc/accounts/lockduration", 300)
            }

            Button {
                text: qsTr("Set")

                onClicked: {
                    settings.setValue("/ipc/accounts/lockduration", unlockDurSpinBox.value)
                }
            }
        }

        Row {
            width: parent.width

            Label {
                text: qsTr("Update interval (s): ")
            }

            SpinBox {
                id: intervalSpinBox
                width: 1 * dpi
                minimumValue: 5
                maximumValue: 60

                value: settings.value("/ipc/interval", 10)
            }

            Button {
                text: qsTr("Set")

                onClicked: {
                    settings.setValue("/ipc/interval", intervalSpinBox.value)
                    ipc.setInterval(intervalSpinBox.value * 1000)
                }
            }
        }

    }
}
