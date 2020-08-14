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
 * @date 2015
 *
 * Transactions tab
 */

import QtQuick 2.12
import QtQuick.Controls 1.4 as C
import QtQuick.Controls 2.12

Loader {
    id: transactionsTab
    enabled: !ipc.busy && !ipc.starting && (ipc.connectionState > 0)

    Column {
        anchors.fill: parent
        anchors.margins: 0.05 * dpi
        anchors.topMargin: 0.1 * dpi
        spacing: 0.1 * dpi

        TransactionDialog {
            id: sendDialog
        }

        TransactionDetails {
            id: details
        }

        Button {
            id: sendButton
            text: "Send Ether"
            width: parent.width
            height: 1 * dpi

            onClicked: sendDialog.display()
        }

        C.TableView {
            id: transactionView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - parent.spacing - sendButton.height

            C.TableViewColumn {
                horizontalAlignment: Text.AlignRight
                role: "blocknumber"
                title: qsTr("Block#")
                width: parent.width * 0.1
            }
            C.TableViewColumn {
                role: "senderalias"
                title: qsTr("Sender")
                width: parent.width * 0.28
            }
            C.TableViewColumn {
                role: "receiveralias"
                title: qsTr("Receiver")
                width: parent.width * 0.28
            }
            C.TableViewColumn {
                horizontalAlignment: Text.AlignRight
                role: "value"
                title: qsTr("Value (Ether)")
                width:  parent.width * 0.2
            }
            C.TableViewColumn {
                role: "depth"
                title: qsTr("Depth")
                width:  parent.width * 0.1
            }
            model: transactionModel

            Menu {
                id: rowMenu

                MenuItem {
                    text: qsTr("Details")
                    onTriggered: {
                        details.display(transactionModel.getJson(transactionView.currentRow, true))
                    }
                }

                MenuItem {
                    text: qsTr("Find on blockchain explorer")
                    onTriggered: {
                        var url = "https://" + (ipc.testnet ? "rinkeby." : "") + "etherscan.io/tx/" + transactionModel.getHash(transactionView.currentRow)
                        Qt.openUrlExternally(url)
                    }
                }

                MenuItem {
                    text: qsTr("Copy Transaction Hash")
                    onTriggered: {
                        clipboard.setText(transactionModel.getHash(transactionView.currentRow))
                    }
                }

                MenuItem {
                    text: qsTr("Copy Sender")
                    onTriggered: {
                        clipboard.setText(transactionModel.getSender(transactionView.currentRow))
                    }
                }

                MenuItem {
                    text: qsTr("Copy Receiver")
                    onTriggered: {
                        clipboard.setText(transactionModel.getReceiver(transactionView.currentRow))
                    }
                }
            }

            onDoubleClicked: {
                if ( transactionView.currentRow >= 0 ) {
                    details.display(transactionModel.getJson(transactionView.currentRow, true))
                }
            }

            MouseArea {
                anchors.fill: parent
                propagateComposedEvents: true
                acceptedButtons: Qt.RightButton

                onReleased: {
                    if ( transactionView.currentRow >= 0 ) {
                        rowMenu.popup()
                    }
                }
            }
        }

    }
}
