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
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0
import AccountProxyModel 0.1
import QtQuick.Dialogs 1.2

Tab {
    id: accountsTab
    enabled: !ipc.busy && !ipc.starting && (ipc.connectionState > 0)
    title: qsTr("Accounts")
    property bool show_hashes: false

    Column {
        id: col
        anchors.margins: 0.05 * dpi
        anchors.topMargin: 0.1 * dpi
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
                enabled: !ipc.syncing && !ipc.closing && !ipc.starting
                text: qsTr("New account")
                onClicked: {
                    accountNewDialog.openFocused("New account password")
                }
            }

            CheckBox {
                id: showHashButton
                anchors.left: newAccountButton.right
                anchors.leftMargin: 0.01 * dpi
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Show Hashes")
                onClicked: {
                    show_hashes = !show_hashes
                }
            }

            Label {
                id: currencyLabel
                anchors.rightMargin: 0.01 * dpi
                anchors.right: currencyCombo.left
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Currency")
            }

            ComboBox {
                id: currencyCombo
                width: 1 * dpi
                anchors.right: totalLabel.left
                anchors.rightMargin: 0.01 * dpi
                anchors.verticalCenter: parent.verticalCenter
                height: newAccountButton.height
                model: currencyModel
                textRole: "name"
                onActivated: {
                    currencyModel.setCurrencyIndex(index);
                }
            }

            Label {
                id: totalLabel
                anchors.right: totalField.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 0.01 * dpi
                text: qsTr("Wallet total: ")
            }

            TextField {
                id: totalField
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: 1 * dpi
                horizontalAlignment: TextInput.AlignRight
                readOnly: true
                text: Number(accountModel.total).toFixed(2)
            }
        }

        AccountDialog {
            id: accountNewDialog

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

        ConfirmDialog {
            id: accountRemoveDialog
            title: qsTr("Confirm removal of account")
            onYes: accountModel.removeAccount(accountModel.selectedAccount)
        }

        QRExportDialog {
            id: qrExportDialog
        }

        FileDialog {
            id: fileExportDialog
            selectFolder: true
            selectExisting: true
            selectMultiple: false
            folder: shortcuts.documents
            onAccepted: {
                if ( accountModel.exportAccount(fileUrl, accountView.currentRow) ) {
                    appWindow.showBadge(qsTr("Account ") + accountModel.selectedAccount + qsTr(" saved to ") + helpers.localURLToString(fileUrl))
                } else {
                    appWindow.showBadge(qsTr("Error exporting account ") + accountModel.selectedAccount)
                }
            }
        }

        AccountDetails {
            id: accountDetails
        }

        TableView {
            id: accountView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - newAccountButton.height - parent.spacing

            TableViewColumn {
                role: "default"
                title: "☑"
                width: 0.3 * dpi
                resizable: false
                horizontalAlignment: Text.AlignHCenter
            }

            TableViewColumn {
                role: "deviceType"
                title: " ⊡"
                width: 0.3 * dpi
                resizable: false
                horizontalAlignment: Text.AlignLeft
            }

            TableViewColumn {
                role: show_hashes ? "hash" : "alias"
                title: qsTr("Account")
                width: 4.75 * dpi
            }

            TableViewColumn {
                horizontalAlignment: Text.AlignRight
                role: "balance"
                title: qsTr("Balance ") + "(" + currencyModel.currencyName + ")"
                width: 2.5 * dpi
            }

            // TODO: fix selection for active row first
            /*sortIndicatorVisible: true
            model: AccountProxyModel {
                   id: proxyModel
                   source: accountModel

                   sortOrder: accountView.sortIndicatorOrder
                   sortCaseSensitivity: Qt.CaseInsensitive
                   sortRole: accountView.getColumn(accountView.sortIndicatorColumn).role

                   filterString: "*"
                   filterSyntax: AccountProxyModel.Wildcard
                   filterCaseSensitivity: Qt.CaseInsensitive
               }*/
            model: accountModel

            Menu {
                id: rowMenu

                MenuItem {
                    text: qsTr("Details", "account")
                    onTriggered: accountDetails.open()
                }

                MenuItem {
                    text: qsTr("Set as default")
                    onTriggered: accountModel.setAsDefault(accountModel.selectedAccount)
                }

                MenuItem {
                    text: qsTr("Alias Account Name")
                    onTriggered: accountRenameDialog.openFocused("Rename " + accountModel.selectedAccount)
                }

                MenuItem {
                    text: qsTr("Copy")
                    onTriggered: clipboard.setText(accountModel.selectedAccount)
                }

                MenuItem {
                    text: qsTr("Find on blockchain explorer")
                    onTriggered: {
                        var url = "http://" + (ipc.testnet ? "rinkeby." : "") + "etherscan.io/address/" + accountModel.selectedAccount
                        Qt.openUrlExternally(url)
                    }
                }

                MenuItem {
                    text: qsTr("Remove", "account")
                    visible: accountModel.selectedAccountHDPath
                    onTriggered: {
                        accountRemoveDialog.msg = qsTr("Remove", "account") + " " + accountModel.selectedAccount + ' <a href="http://www.etherwall.com/faq/#removeaccount">?</a>'
                        accountRemoveDialog.open()
                    }
                }

                MenuItem {
                    visible: !accountModel.selectedAccountHDPath
                    text: qsTr("Export geth account to directory")
                    onTriggered: fileExportDialog.open(helpers.exportAddress(accountModel.selectedAccount, ipc.testnet))
                }

                MenuItem {
                    visible: !accountModel.selectedAccountHDPath
                    text: qsTr("Export geth account to QR Code")
                    onTriggered: qrExportDialog.open(helpers.exportAddress(accountModel.selectedAccount, ipc.testnet), accountModel.selectedAccount)
                }
            }

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
