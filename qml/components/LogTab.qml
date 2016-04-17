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
/** @file AccountsTab.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Log tab
 */

import QtQuick 2.0
import QtQuick.Controls 1.4

Tab {
    id: logTab
    title: qsTr("Logs")

    Column {
        id: col
        anchors.margins: 0.2 * dpi
        anchors.fill: parent

        Row {
            id: logControlRow

            Label {
                text: qsTr("Log level: ")
            }

            ComboBox {
                id: logLevelCombo
                currentIndex: log.logLevel

                onCurrentIndexChanged: log.logLevel = currentIndex

                model: ListModel {
                    id: llItems
                    ListElement { text: "Debug"; value: 0 }
                    ListElement { text: "Info"; value: 1 }
                    ListElement { text: "Warning"; value: 2 }
                    ListElement { text: "Error"; value: 3 }
                }
            }

            Button {
                id: logClipButton
                text: "Save to clipboard"
                onClicked: {
                    log.saveToClipboard()
                }
            }
        }

        TableView {
            id: logView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - parent.spacing - logControlRow.height

            TableViewColumn {
                role: "date"
                title: qsTr("Date")
                width: 2 * dpi
            }
            TableViewColumn {
                role: "severity"
                title: qsTr("Severity")
                width: 1 * dpi
            }
            TableViewColumn {
                role: "msg"
                title: qsTr("Message")
                width: 4 * dpi
            }
            model: log

            rowDelegate: Item {
                height: 1 * dpi
                SystemPalette {
                    id: osPalette
                    colorGroup: SystemPalette.Active
                }

                Rectangle {
                    anchors {
                        left: parent.left
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }
                    height: parent.height
                    color: styleData.selected ? osPalette.highlight : (styleData.alternate ? osPalette.alternateBase : osPalette.base)
                }
            }

            itemDelegate: TextArea {
                readOnly: true
                text: styleData.value
            }
        }
    }
}
