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
import QtQuick.Controls.Styles 1.1
import QtQuick.Window 2.0

Window {
    id: contractCalls
    title: qsTr("Call Contract")

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

    property int contractIndex : -1

    function open( index ) {
        contractIndex = index
        show()
    }

    BusyIndicator {
        anchors.centerIn: parent
        z: 10
        running: ipc.starting || ipc.busy || ipc.syncing
    }

    Column {
        id: mainColumn
        anchors.fill: parent
        anchors.margins: 0.1 * dpi
        spacing: 0.2 * dpi

        Row {
            Label {
                width: 1 * dpi
                text: qsTr("Name: ")
            }

            TextField {
                id: nameField
                width: mainColumn.width - 1 * dpi
                text: contractModel.getName(contractIndex)
                readOnly: true
            }
        }

        Row {
            Label {
                width: 1 * dpi
                text: qsTr("Function: ")
            }

            ComboBox {
                id: functionField
                width: mainColumn.width - 1 * dpi
                model: contractModel.getFunctions(contractIndex)

                onCurrentTextChanged: {
                    argsView.params = []
                    argsView.model = contractModel.getArguments(contractIndex, currentText)
                }
            }
        }

        ListView {
            id: argsView
            width: parent.width
            height: 2 * dpi
            property variant params : []

            delegate: Row {
                Label {
                    width: 1 * dpi
                    text: modelData
                }

                TextField {
                    width: mainColumn.width - 1 * dpi
                    onTextChanged: {
                        argsView.params[index] = text
                    }
                }
            }
        }

        TextArea {
            id: encoded
            width: parent.width
            readOnly: true
            wrapMode: TextEdit.WrapAnywhere
            height: 0.5 * dpi
        }

        Button {
            id: callButton
            width: parent.width
            height: 0.6 * dpi
            text: "Save"

            Image {
                id: callIcon
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: parent.height * 0.15
                width: height
                source: "/images/warning"
            }

            style: ButtonStyle {
              label: Text {
                renderType: Text.NativeRendering
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: callButton.height / 2.0
                text: control.text
              }
            }

            function check() {
                var result = {
                    error: null
                }

                return result;
            }

            function refresh() {
                var result = check()
                if ( result.error !== null ) {
                    tooltip = result.error
                    callIcon.source = "/images/warning"
                    return result
                }

                callIcon.source = "/images/ok"
                return result
            }

            onClicked: {
                var result = refresh()
                if ( result.error !== null ) {
                    errorDialog.msg = result.error
                    errorDialog.open()
                    return
                }

                try {
                    encoded.text = contractModel.encodeCall(contractIndex, functionField.currentText, argsView.params);
                } catch ( err ) {
                    console.error(err)
                }
            }
        }

        Button {
            text: qsTr("Close")
            width: parent.width
            height: 0.6 * dpi

            onClicked: {
                contractCalls.close()
            }
        }

    }
}
