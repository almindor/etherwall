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
/** @file PinMatrixDialog.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2017
 *
 * PinMatrixDialog dialog
 */

import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 1.2

Window {
    id: pinMatrixDialog
    title: qsTr("Enter your TREZOR pin")

    property string pin : ""
    property bool accepted : false

    modality: Qt.ApplicationModal
    visible: false
    minimumWidth: 3 * dpi
    minimumHeight: 3 * dpi
    maximumWidth: 8 * dpi
    maximumHeight: 8 * dpi
    width: 4 * dpi
    height: 5 * dpi
    Component.onCompleted: {
        setX(Screen.width / 2.0 - width / 2.0)
        setY(Screen.height / 2.0 - height / 2.0)
    }

    onVisibleChanged: {
        if ( !visible && !accepted ) {
            trezor.cancel()
        } else if ( visible ) {
            accepted = false
        }
    }

    function open() {
        pin = ""
        pinEdit.text = ""
        show()
    }

    ListModel {
        id: numericModel
        ListElement {
            val: "7"
        }
        ListElement {
            val: "8"
        }
        ListElement {
            val: "9"
        }
        ListElement {
            val: "4"
        }
        ListElement {
            val: "5"
        }
        ListElement {
            val: "6"
        }
        ListElement {
            val: "1"
        }
        ListElement {
            val: "2"
        }
        ListElement {
            val: "3"
        }
    }

    Component {
        id: contactDelegate
        Button {
            width: grid.cellWidth
            height: grid.cellHeight
            text: "*"
            onClicked: {
                pin += val
                pinEdit.text += '*';
            }
        }
    }

    Column {
        Keys.onPressed: {
            if ( event.key >= Qt.Key_1 && event.key <= Qt.Key_9 ) {
                pin += event.text
                pinEdit.text += '*';
            } else if ( event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete ) {
                var l = pin.length - 1
                pin = pin.substring(0, l)
                pinEdit.text = pinEdit.text.substring(0, l)
            } else if ( event.key === Qt.Key_Return || event.key === Qt.Key_Enter ) {
                accepted = true
                trezor.submitPin(pin)
                pin = ""
                pinMatrixDialog.close()
            }
        }

        anchors {
            left: parent.left
            right: parent.right
        }

        Item {
            width: parent.width
            height: pinEdit.height

            TextEdit {
                id: pinEdit
                font.pixelSize: 0.5 * dpi
                anchors {
                    left: parent.left
                    right: backButton.left
                }

                readOnly: true
                text: ""
            }

            Button {
                id: backButton
                anchors {
                    right: parent.right
                }

                height: pinEdit.height
                text: "<"
                onClicked: {
                    var l = pin.length - 1
                    pin = pin.substring(0, l)
                    pinEdit.text = pinEdit.text.substring(0, l)
                }
            }
        }

        GridView {
            id: grid
            anchors {
                left: parent.left
                right: parent.right
            }
            height: pinMatrixDialog.height - pinEdit.height - submitButton.height

            cellWidth: width / 3.0
            cellHeight: height / 3.0

            model: numericModel
            delegate: contactDelegate
            highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
            focus: true
        }

        Button {
            id: submitButton
            anchors {
                left: parent.left
                right: parent.right
            }

            height: 0.5 * dpi
            text: qsTr("Submit")
            onClicked: {
                accepted = true
                trezor.submitPin(pin)
                pin = ""
                pinMatrixDialog.close()
            }
        }
    }
}
