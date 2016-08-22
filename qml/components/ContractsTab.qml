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
/** @file TransactionsTab.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * Contracts tab
 */

import QtQuick 2.0
import QtQuick.Controls 1.1

Tab {
    id: constractsTab
    enabled: !ipc.busy && !ipc.starting && (ipc.connectionState > 0)
    title: qsTr("Contracts")

    Column {
        anchors.fill: parent
        anchors.margins: 0.05 * dpi
        anchors.topMargin: 0.1 * dpi
        spacing: 0.1 * dpi

        ContractDetails {
            id: details
        }

        ContractCalls {
            id: calls
        }

        Button {
            text: "Add Contract"
            width: parent.width
            height: 1 * dpi

            onClicked: details.open(-1)
        }

        TableView {
            id: contractView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - parent.spacing

            TableViewColumn {
                role: "name"
                title: qsTr("Name")
                width: 2.25 * dpi
            }
            TableViewColumn {
                role: "address"
                title: qsTr("Address")
                width: 5 * dpi
            }
            model: contractModel

            Menu {
                id: rowMenu

                MenuItem {
                    text: qsTr("Call")
                    onTriggered: {
                        calls.open(contractView.currentRow)
                    }
                }

                MenuItem {
                    text: qsTr("Edit")
                    onTriggered: {
                        details.open(transactionModel.getJson(contractView.currentRow, true))
                    }
                }

                MenuItem {
                    text: qsTr("Find on blockchain explorer")
                    onTriggered: {
                        var url = "http://" + (ipc.testnet ? "testnet." : "") + "etherscan.io/address/" + contractModel.getAddress(contractView.currentRow)
                        Qt.openUrlExternally(url)
                    }
                }

                MenuItem {
                    text: qsTr("Copy Address")
                    onTriggered: {
                        clipboard.setText(contractModel.getAddress(contractView.currentRow))
                    }
                }

                MenuItem {
                    text: qsTr("Delete")
                    onTriggered: {
                        contractModel.deleteContract(contractView.currentRow)
                    }
                }
            }

            rowDelegate: Item {
                SystemPalette {
                    id: osPalette
                    colorGroup: SystemPalette.Active
                }

                height: 0.2 * dpi

                Rectangle {
                    anchors {
                        left: parent.left
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }
                    height: parent.height
                    color: styleData.selected ? osPalette.highlight : (styleData.alternate ? osPalette.alternateBase : osPalette.base)
                    MouseArea {
                        anchors.fill: parent
                        propagateComposedEvents: true
                        acceptedButtons: Qt.RightButton

                        onReleased: {
                            if ( contractView.currentRow >= 0 ) {
                                rowMenu.popup()
                            }
                        }
                    }
                }
            }
        }

    }
}
