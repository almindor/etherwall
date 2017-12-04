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
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.1

Dialog {
    title: qsTr("First time setup wizard")
    standardButtons: StandardButton.Save | StandardButton.Close
    modality: Qt.ApplicationModal
    visible: false
    width: 7 * dpi
    height: 5 * dpi

    onAccepted: {
        settings.setValue("program/v2firstrun", new Date())
        initializer.start();
    }

    Column {
        anchors.margins: 0.1 * dpi
        anchors.fill: parent
        spacing: 0.1 * dpi

        Text {
            id: info0
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 0.1 * dpi
            font.pixelSize: 0.16 * dpi
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            text: qsTr("Please review settings before first run.")
        }

        Text {
            id: info1
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 0.1 * dpi
            font.pixelSize: 0.16 * dpi
            font.bold: true
            textFormat: Text.RichText
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            onLinkActivated: Qt.openUrlExternally(link)
            text: qsTr("Accounts are stored in the geth datadir folder! <a href=\"https://www.etherwall.com/faq/#accounts\">Click here for more info</a>.")
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
                cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
            }
        }

        Item {
            id: info2
            height: 0.5 * dpi
            anchors {
                left: parent.left
                right: parent.right
            }

            Text {
                visible: content.thinClient
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 0.1 * dpi
                font.pixelSize: 0.16 * dpi
                font.bold: true
                textFormat: Text.RichText
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                onLinkActivated: Qt.openUrlExternally(link)
                text: qsTr("Thin client is recommended due to chaindata size. <a href=\"https://www.etherwall.com/faq/#thinclient\">Click here for more info</a>.")
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }

            Text {
                visible: !content.thinClient
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 0.1 * dpi
                font.pixelSize: 0.16 * dpi
                font.bold: true
                textFormat: Text.RichText
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                onLinkActivated: Qt.openUrlExternally(link)
                text: qsTr("Ethereum blockchain requires at least 40GB of space and takes a long time to synchronize. Use of thin client is preferred. <a href=\"https://www.etherwall.com/faq/#thinclient\">Click here for more info</a>.")
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }
        }

        SettingsContent {
            id: content
            anchors.left: parent.left
            anchors.right: parent.right
            height: 3 * dpi
            hideTrezor: true
        }
    }
}
