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
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.0

Tab {
    id: accountsTab
    enabled: !ipc.busy && (ipc.connectionState > 0)
    title: qsTr("Accounts")
    property bool show_hashes: false

    Column {
        id: col
        anchors.margins: 20
        anchors.fill: parent

        Item {
            id: rowHeader
            anchors.left: parent.left
            anchors.right: parent.right
            height: newAccountButton.height
            Button {
                id: newAccountButton
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                text: qsTr("New account")
                onClicked: {
                    accountNewDialog.openFocused("New account password")
                }
            }

            Switch {
                id: showHashButton
                anchors.left: newAccountButton.right
                anchors.leftMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    show_hashes = !show_hashes
                }
            }

            Label {
                id: labelHashes
                anchors.leftMargin: 5
                anchors.left: showHashButton.right
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Show hashes")
            }

            Label {
                id: currencyLabel
                anchors.rightMargin: 5
                anchors.right: currencyCombo.left
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Currency")
            }

            ComboBox {
                id: currencyCombo
                width: 70
                anchors.right: totalLabel.left
                anchors.rightMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                height: newAccountButton.height
                model: ["ETH", "USD", "CAD", "EUR", "GBP"]
                onCurrentIndexChanged: {} // TODO
            }

            Label {
                id: totalLabel
                anchors.right: totalField.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 5
                text: qsTr("Wallet total ")
            }

            TextField {
                id: totalField
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                height: newAccountButton.height
                width: 210
                horizontalAlignment: TextInput.AlignRight
                readOnly: true
                text: accountModel.total
            }
        }

        PasswordDialog {
            id: accountNewDialog
            acceptEmpty: false

            onAccepted: {
                accountModel.newAccount(password)
            }
        }

        InputDialog {
            id: accountRenameDialog
            query: qsTr("Account Alias: ")
            //standardButtons: StandardButton.Ok | StandardButton.Cancel

            onAccepted: {
                accountModel.renameAccount(value, accountView.currentRow);
                transactionModel.lookupAccountsAliases();
            }
        }

        PasswordDialog {
            id: accountDeleteDialog
            //standardButtons: StandardButton.Ok | StandardButton.Cancel

            onAccepted: {
                accountModel.deleteAccount(password, accountView.currentRow);
            }
        }

        PasswordDialog {
            id: accountUnlockDialog
            //standardButtons: StandardButton.Ok | StandardButton.Cancel

            onAccepted: {
                accountModel.unlockAccount(password, settings.value("ipc/accounts/lockduration", 300), accountView.currentRow)
            }
        }

        TableView {
            id: accountView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - newAccountButton.height - parent.spacing

            TableViewColumn {
                horizontalAlignment: Text.AlignHCenter
                role: "locked"
                title: qsTr("Locked")
                width: 70
                delegate: ToolButton {
                    iconSource: (styleData.value === true) ? "/images/locked" : "/images/unlocked"
                    enabled: (styleData.value === true)
                    onClicked: {
                        if ( styleData.value === true ) {
                            accountView.currentRow = styleData.row
                            accountModel.selectedAccountRow = accountView.currentRow
                            accountUnlockDialog.openFocused("Unlock " + accountModel.selectedAccount)
                        }
                    }
                }
            }
            TableViewColumn {
                role: show_hashes ? "hash" : "alias"
                title: qsTr("Account")
                width: 400
            }
            TableViewColumn {
                horizontalAlignment: Text.AlignRight
                role: "balance"
                title: qsTr("Balance (Ether)")
                width: 150
            }
            TableViewColumn {
                horizontalAlignment: Text.AlignRight
                role: "transactions"
                title: qsTr("Sent Trans.")
                width: 100
            }
            model: accountModel

            Menu {
                id: rowMenu

                MenuItem {
                    text: qsTr("Alias Account Name")
                    onTriggered: {
                        accountRenameDialog.openFocused("Rename " + accountModel.selectedAccount)
                    }
                }

                MenuItem {
                    text: qsTr("Copy")
                    onTriggered: {
                        clipboard.setText(accountModel.selectedAccount)
                    }
                }

                /*MenuItem {
                    text: qsTr("Delete")
                    onTriggered: {
                        accountDeleteDialog.openFocused("Delete " + accountModel.selectedAccount)
                    }
                }*/
            }

            rowDelegate: Item {
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
                    MouseArea {
                        anchors.fill: parent
                        propagateComposedEvents: true
                        acceptedButtons: Qt.RightButton

                        onReleased: {
                            if ( accountView.currentRow >= 0 ) {
                                accountModel.selectedAccountRow = accountView.currentRow
                                rowMenu.popup()
                            }
                        }
                    }
                }
            }
        }
    }
}
