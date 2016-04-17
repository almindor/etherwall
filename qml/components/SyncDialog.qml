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
/** @file ConfirmDialog.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * Sync dialog
 */

import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Window 2.0

Window {
    id: syncWindow
    title: qsTr("Geth synchronizing blocks")
    modality: Qt.ApplicationModal
    visible: false
    minimumWidth: 4 * dpi
    minimumHeight: 0.5 * dpi
    maximumWidth: 8 * dpi
    maximumHeight: 4 * dpi
    //flags: Qt.CustomizeWindowHint | Qt.WindowTitleHint | Qt.WindowCloseButtonHint
    width: 5 * dpi
    height: 1 * dpi
    Component.onCompleted: {
        setX(Screen.width / 2.0 - width / 2.0)
        setY(Screen.height / 2.0 - height / 2.0)
    }

    Column {
        id: syncCol
        anchors.margins: 0.1 * dpi
        anchors.fill: parent
        //spacing: 0 * dpi

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Synchronized ") + Math.max(0, ipc.currentBlock - ipc.startingBlock) + qsTr(" out of ") + Math.max(0, ipc.highestBlock - ipc.startingBlock) + qsTr(" blocks")
        }

        ProgressBar {
            anchors.left: parent.left
            anchors.right: parent.right
            value: Math.max(0, ipc.currentBlock - ipc.startingBlock) / Math.max(1, ipc.highestBlock - ipc.startingBlock)
        }

        Button {
            id: quitButton
            text: qsTr("Quit", "Syncing dialog - quit app")
            anchors.right: parent.right
            onClicked: {
                syncWindow.close()
                appWindow.close()
            }
        }
    }
}
