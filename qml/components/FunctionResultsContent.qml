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
/** @file FunctionResultContent.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2017
 *
 * Function results content
 */

import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Controls 1.4 as C

Item {
    id: itemID
    anchors.fill: parent
    anchors.topMargin: 0.1 * dpi
    property int callIndex : -1
    signal done()

    Connections {
        target: ipc

        function onCallDone(result, index, userData) {
            if ( userData["type"] === "functionCall" ) {
                responseField.model = contractModel.parseResponse(index, result, userData)
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        z: 10
        running: ipc.starting || ipc.busy || ipc.syncing
    }

    C.TableView {
        id: responseField
        anchors.fill: itemID
        // headerVisible: true

        C.TableViewColumn {
            role: "number"
            title: "#"
            width: 0.5 * dpi
        }

        C.TableViewColumn {
            role: "type"
            title: qsTr("Type")
            width: 1 * dpi
        }

        C.TableViewColumn {
            role: "value"
            title: qsTr("Value")
            width: responseField.width - 2.6 * dpi
        }

        Menu {
            id: rowMenu

            MenuItem {
                text: qsTr("Copy Value")
                onTriggered: {
                    if ( responseField.currentRow >= 0 ) {
                        clipboard.setText(responseField.model[0].value)
                    }
                }
            }

        }

        MouseArea {
            anchors.fill: parent
            propagateComposedEvents: true
            acceptedButtons: Qt.RightButton

            onReleased: {
                if ( responseField.currentRow >= 0 ) {
                    rowMenu.popup()
                }
            }
        }
    }
}
