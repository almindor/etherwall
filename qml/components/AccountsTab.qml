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

import QtQuick 2.12
import Qt.labs.platform 1.0
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.12
import AccountProxyModel 0.1

Loader {
    id: accountsTab
    enabled: !ipc.busy && !ipc.starting && (ipc.connectionState > 0)
    // property bool show_hashes: false

    Column {
        id: col
        anchors.margins: 0.05 * dpi
        anchors.topMargin: 0.1 * dpi
        anchors.fill: parent

        Item {
            id: rowHeader
            anchors.left: parent.left
            anchors.right: parent.right
            height: newAccountButton.height + 0.1 * dpi
            Button {
                id: newAccountButton
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                enabled: !ipc.syncing && !ipc.closing && !ipc.starting
                text: qsTr("New account")
                onClicked: {
                    accountNewDialog.open()
                }
            }

            Label {
                id: tokenLabel
                anchors.rightMargin: 0.01 * dpi
                anchors.right: tokenCombo.left
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Token")
            }

            ComboBox {
                id: tokenCombo
                width: 1 * dpi
                anchors.right: currencyLabel.left
                anchors.rightMargin: 0.01 * dpi
                anchors.verticalCenter: parent.verticalCenter
                height: newAccountButton.height
                model: tokenModel
                currentIndex: tokenModel.outerIndex
                textRole: "token"
                onActivated: {
                    currencyCombo.currentIndex = 0
                    currencyModel.setCurrencyIndex(0)
                    tokenModel.selectToken(index)
                }
            }

            Label {
                id: currencyLabel
                enabled: tokenCombo.currentIndex === 0
                anchors.rightMargin: 0.01 * dpi
                anchors.right: currencyCombo.left
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Currency")
            }

            ComboBox {
                id: currencyCombo
                enabled: tokenCombo.currentIndex === 0
                width: 1 * dpi
                anchors.right: totalLabel.left
                anchors.rightMargin: 0.01 * dpi
                anchors.verticalCenter: parent.verticalCenter
                height: newAccountButton.height
                model: currencyModel
                textRole: "name"
                onActivated: currencyModel.setCurrencyIndex(index);
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

            onValidPassword: accountModel.newAccount(password)
            onInvalidPassword: {
                errorDialog.text = qsTr("Password mismatch or invalid password")
                errorDialog.open()
            }
        }

        InputDialog {
            id: accountRenameDialog
            query: qsTr("Account Alias: ")

            onAcceptedInput: {
                accountModel.renameAccount(value, accountView.currentRow);
                transactionModel.lookupAccountsAliases();
            }
        }

        Dialog {
            id: accountRemoveDialog
            property string text : ""
            title: qsTr("Confirm removal of account")
            Text {
                text: accountRemoveDialog.text
            }

            standardButtons: Dialog.Yes | Dialog.No | Dialog.Help
            onAccepted: accountModel.removeAccount(accountModel.selectedAccount)
            onHelpRequested: Qt.openUrlExternally("https://www.etherwall.com/faq/#removeaccount")
        }

        QRExportDialog {
            id: qrExportDialog
        }

        FolderDialog {
            id: fileExportDialog
            onAccepted: {
                if ( accountModel.exportAccount(folder, accountView.currentRow) ) {
                    appWindow.showBadge(qsTr("Account ") + accountModel.selectedAccount + qsTr(" saved to ") + helpers.localURLToString(folder))
                } else {
                    appWindow.showBadge(qsTr("Error exporting account ") + accountModel.selectedAccount)
                }
            }
        }

        AccountDetails {
            id: accountDetails
        }

        HorizontalHeaderView {
            syncView: accountView
            model: ["D", "T", "Alias", "Hash", "Balance"]
        }

        TableView {
            id: accountView
            anchors.left: parent.left
            anchors.right: parent.right
            implicitHeight: parent.height - newAccountButton.height - parent.spacing // *
            onWidthChanged: forceLayout()
            columnWidthProvider: function (column) { // *
                switch (column) {
                    case 0: return 0.2 * dpi
                    case 1: return 0.2 * dpi
                    case 2: return width - 7.4 * dpi
                    case 3: return 4.5 * dpi
                    case 4: return 2.5 * dpi
                }

                return 0
            }

            property int currentRow: -1

            delegate: Rectangle {
                implicitWidth: cellText.width + 0.2 * dpi
                implicitHeight: 0.5 * dpi
                color: row === accountView.currentRow ? Universal.baseLowColor : Universal.altLowColor
                border {
                    color: Universal.chromeBlackLowColor
                    width: 1
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: accountView.currentRow = row
                    onDoubleClicked: if ( accountView.currentRow >= 0 ) { // *
                        accountModel.selectedAccountRow = accountView.currentRow
                        accountDetails.open(accountView.currentRow)
                    }
                }

                Text {
                    id: cellText
                    anchors.centerIn: parent
                    text: display
                }
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
            model: accountModel // *

            Menu { // *
                id: rowMenu
                enabled: accountView.currentRow >= 0

                MenuItem {
                    text: qsTr("Details", "account")
                    onTriggered: accountDetails.open(accountModel.selectedAccountRow)
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
                        var url = "https://" + (ipc.testnet ? "rinkeby." : "") + "etherscan.io/address/" + accountModel.selectedAccount
                        Qt.openUrlExternally(url)
                    }
                }

                MenuItem {
                    text: qsTr("Remove", "account")
                    enabled: accountModel.selectedAccountHDPath
                    onTriggered: {
                        accountRemoveDialog.text = qsTr("Remove", "account") + " " + accountModel.selectedAccount + '?'
                        accountRemoveDialog.open()
                    }
                }

                MenuItem {
                    enabled: !accountModel.selectedAccountHDPath
                    text: qsTr("Export geth account to directory")
                    onTriggered: fileExportDialog.open(helpers.exportAddress(accountModel.selectedAccount, ipc.testnet))
                }

                MenuItem {
                    enabled: !accountModel.selectedAccountHDPath
                    text: qsTr("Export geth account to QR Code")
                    onTriggered: qrExportDialog.display(helpers.exportAddress(accountModel.selectedAccount, ipc.testnet), accountModel.selectedAccount)
                }
            }

            MouseArea { // *
                anchors.fill: parent
                propagateComposedEvents: true
                acceptedButtons: Qt.RightButton

                onReleased: {
                    accountModel.selectedAccountRow = accountView.currentRow
                    rowMenu.popup()
                }
            }
        }
    }
}
