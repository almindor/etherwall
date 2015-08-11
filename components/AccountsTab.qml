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
 * Accounts tab
 */

import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Dialogs 1.2

Tab {
    id: accountsTab
    title: qsTr("Accounts")

    Column {
        id: col
        anchors.fill: parent

        Row {
            width: parent.width

            Button {
                id: newAccountButton
                text: qsTr("New account")
                onClicked: {
                    accountNewDialog.open()
                }
            }

            Button {
                id: deleteAccountButton
                text: qsTr("Delete account")
                enabled: (accountView.currentRow >= 0)

                onClicked: {
                    accountDeleteDialog.open()
                }
            }
        }

        AccountDialog {
            id: accountNewDialog
            standardButtons: StandardButton.Save | StandardButton.Cancel

            onAccepted: {
                console.log("NEW: " + accountModel.newAccount(accountNewDialog.password))
            }
        }

        AccountDialog {
            id: accountDeleteDialog
            standardButtons: StandardButton.Ok | StandardButton.Cancel

            onAccepted: {
                console.log("DELETE: " + accountModel.deleteAccount(accountView.currentRow, accountDeleteDialog.password))
            }
        }

        TableView {
            id: accountView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - newAccountButton.height - parent.spacing

            TableViewColumn {
                role: "hash"
                title: qsTr("Hash")
                width: 400
            }
            TableViewColumn {
                role: "balance"
                title: qsTr("Balance (Ether)")
                width: 150
            }
            TableViewColumn {
                role: "transactions"
                title: qsTr("Transactions Sent")
                width: 150
            }
            model: accountModel
        }
    }
}
