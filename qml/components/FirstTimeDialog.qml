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
    maximumHeight: 7 * dpi
    width: 7 * dpi
    x: Screen.width / 2.0 - width / 2.0
    y: Screen.height / 2.0 - height / 2.0

    Column {
        anchors.margins: 0.1 * dpi
        anchors.fill: parent
        spacing: 0.25 * dpi

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            text: qsTr("Please confirm options for Geth before running for the first time")
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            text: qsTr("NOTE: the --datadir option might be needed if your main drive has less than 40GB of free space")
        }

        Row {
            id: rowGethArgs
            width: parent.width

            Label {
                id: gethArgsLabel
                text: "Geth args: "
            }

            TextField {
                id: gethArgsField
                width: parent.width - gethArgsLabel.width
                text: settings.value("geth/args", "--fast --cache 512")
            }
        }

        Button {
            id: quitButton
            text: qsTr("Continue", "First time dialog")
            anchors.right: parent.right
            onClicked: {
                settings.setValue("geth/args", gethArgsField.text)
                settings.setValue("program/firstrun", new Date())
                ftWindow.close()
                ipc.init();
            }
        }
    }
}
