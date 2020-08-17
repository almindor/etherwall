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
/** @file EventDetails.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * Filter Details dialog
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls 1.4 as C
import QtQuick.Controls.Styles 1.4

Dialog {
    id: eventDetails
    title: qsTr("Event Details")
    standardButtons: Dialog.Close
    // modality: Qt.WindowModal
    visible: false
    width: 7 * dpi
    height: 7 * dpi
    focus: true
    anchors.centerIn: parent

    function display( index ) {
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

        open()
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

        C.TableView {
            id: argsField
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1 * dpi

            C.TableViewColumn {
                role: "name"
                title: qsTr("Name")
                width: 1 * dpi
            }

            C.TableViewColumn {
                role: "type"
                title: qsTr("Type")
                width: 1 * dpi
            }

            C.TableViewColumn {
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
    }
}
