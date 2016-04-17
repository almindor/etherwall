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
/** @file FirstTimeDialog.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * FirstTime dialog
 */

import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Window 2.0

Window {
    id: ftWindow
    title: qsTr("First time setup wizard")

    modality: Qt.ApplicationModal
    visible: false
    minimumWidth: 6 * dpi
    minimumHeight: 1 * dpi
    maximumWidth: 10 * dpi
    maximumHeight: 8 * dpi
    width: 7 * dpi
    height: 5 * dpi
    Component.onCompleted: {
        setX(Screen.width / 2.0 - width / 2.0)
        setY(Screen.height / 2.0 - height / 2.0)
    }


    property bool done: false

    Column {
        anchors.margins: 0.1 * dpi
        anchors.fill: parent
        spacing: 0.25 * dpi

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 0.2 * dpi
            font.pixelSize: 0.2 * dpi
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            text: qsTr("Please confirm options for Geth before running for the first time")
        }

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 0.2 * dpi
            font.pixelSize: 0.2 * dpi
            font.bold: true
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            text: qsTr("Ethereum blockchain grows rapidly and requires at least 20GB of space. Make sure to choose an appropriate data directory with enough space left.")
        }

        SettingsContent {
            anchors.left: parent.left
            anchors.right: parent.right
        }

        Row {
            anchors.right: parent.right

            Button {
                id: continueButton
                text: qsTr("Continue", "First time dialog")
                anchors.margins: 0.1 * dpi
                width: 1 * dpi
                height: 0.6 * dpi

                onClicked: {
                    ipc.init();
                    settings.setValue("program/firstrun", new Date())
                    ftWindow.close()
                }
            }

            Button {
                id: quitButton
                text: qsTr("Quit", "First time dialog")
                anchors.margins: 0.1 * dpi
                width: 1 * dpi
                height: 0.6 * dpi

                onClicked: {
                    ftWindow.close()
                    appWindow.close()
                }
            }
        }

    }
}
