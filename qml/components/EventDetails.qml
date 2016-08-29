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
 * Filter Details dialog
 */

import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Window 2.0

Window {
    id: eventDetails
    title: qsTr("Event Details")

    modality: Qt.NonModal
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

    function open( index ) {
        if ( index >= 0 ) {
            nameField.text = eventModel.getName(index);
            contractField.text = eventModel.getContract(index);
            dataField.text = eventModel.getData(index);
            addressField.text = eventModel.getAddress(index);
            blockNumField.text = eventModel.getBlockNumber(index);
            blockHashField.text = eventModel.getBlockHash(index);
            transactionHashField.text = eventModel.getTransactionHash(index);
            topicsField.text = eventModel.getTopics(index);
            argsField.model = eventModel.getArgModel(index);
        }

        show()
    }

    BusyIndicator {
        anchors.centerIn: parent
        z: 10
        running: ipc.starting || ipc.busy || ipc.syncing
    }

    Column {
        id: mainColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 0.1 * dpi
        spacing: 0.1 * dpi

        Label {
            text: qsTr("Event Parameters")
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
        }

        TableView {
            id: argsField
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1 * dpi

            TableViewColumn {
                role: "name"
                title: qsTr("Name")
                width: 1 * dpi
            }

            TableViewColumn {
                role: "type"
                title: qsTr("Type")
                width: 1 * dpi
            }

            TableViewColumn {
                role: "value"
                title: qsTr("Value")
                width: argsField.width - 2.1 * dpi
            }

            Menu {
                id: rowMenu

                MenuItem {
                    text: qsTr("Copy Value")
                    onTriggered: {
                        if ( argsField.currentRow >= 0 ) {
                            clipboard.setText(eventModel.getParamValue(argsField.currentRow))
                        }
                    }
                }

            }

            MouseArea {
                anchors.fill: parent
                propagateComposedEvents: true
                acceptedButtons: Qt.RightButton

                onReleased: {
                    if ( argsField.currentRow >= 0 ) {
                        rowMenu.popup()
                    }
                }
            }
        }

        Row {
            Label {
                width: 1 * dpi
                text: qsTr("Name: ")
            }

            TextField {
                id: nameField
                readOnly: true
                width: mainColumn.width - 1 * dpi

                maximumLength: 255
            }
        }

        Row {
            Label {
                width: 1 * dpi
                text: qsTr("Contract: ")
            }

            TextField {
                id: contractField
                readOnly: true
                width: mainColumn.width - 1 * dpi

                maximumLength: 255
            }
        }

        Row {
            Label {
                text: qsTr("Data: ")
                width: 1 * dpi
            }

            TextField {
                id: dataField
                readOnly: true
                width: mainColumn.width - 1 * dpi

                maximumLength: 255
            }
        }

        Row {
            Label {
                text: qsTr("Address: ")
                width: 1 * dpi
            }

            TextField {
                id: addressField
                readOnly: true
                width: mainColumn.width - 1 * dpi

                maximumLength: 255
            }
        }

        Row {
            Label {
                text: qsTr("Block #: ")
                width: 1 * dpi
            }

            TextField {
                id: blockNumField
                readOnly: true
                width: mainColumn.width - 1 * dpi

                maximumLength: 255
            }
        }

        Row {
            Label {
                text: qsTr("Block hash: ")
                width: 1 * dpi
            }

            TextField {
                id: blockHashField
                readOnly: true
                width: mainColumn.width - 1 * dpi

                maximumLength: 255
            }
        }

        Row {
            Label {
                text: qsTr("Transaction: ")
                width: 1 * dpi
            }

            TextField {
                id: transactionHashField
                readOnly: true
                width: mainColumn.width - 1 * dpi

                maximumLength: 255
            }
        }

        Row {
            Label {
                text: qsTr("Topics: ")
                width: 1 * dpi
            }

            TextField {
                id: topicsField
                readOnly: true
                width: mainColumn.width - 1 * dpi

                maximumLength: 255
            }
        }

        Button {
            id: closeButton
            text: qsTr("Close")
            width: parent.width
            height: 0.6 * dpi

            onClicked: {
                eventDetails.close()
            }
        }

    }
}
