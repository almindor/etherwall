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

import QtQuick 2.12
import QtQuick.Controls 2.15

Dialog {
    title: qsTr("First time setup wizard")
    standardButtons: Dialog.Save | Dialog.Close
    // modality: Qt.ApplicationModal
    visible: false
    width: 9 * dpi
    height: appWindow.height - 0.5 * dpi
    focus: true
    anchors.centerIn: parent

    onAccepted: {
        settings.setValue("program/v2firstrun", new Date())
        initializer.start();
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: 8.7 * dpi
        contentHeight: 7.5 * dpi

        Column {
            id: infoCol
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 0.1 * dpi

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
                text: qsTr("Full node mode is not recommended due to chaindata size. <a href=\"https://www.etherwall.com/faq/#thinclient\">Click here for more info</a>.")
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
                text: qsTr("Ethereum blockchain requires a ludicrous amount of space and takes a long time to synchronize. Use of thin client is preferred. <a href=\"https://www.etherwall.com/faq/#thinclient\">Click here for more info</a>.")
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }
        }

        SettingsContent {
            id: content
            anchors.top: infoCol.bottom
            anchors.topMargin: 0.2 * dpi
            anchors.left: parent.left
            anchors.right: parent.right
            hideTrezor: true
        }
    }
}
