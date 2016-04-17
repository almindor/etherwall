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

import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.0

Tab {
    id: transactionsTab
    enabled: !ipc.busy && !ipc.starting && (ipc.connectionState > 0)
    title: qsTr("Transactions")

    Column {
        anchors.fill: parent
        anchors.margins: 0.05 * dpi
        anchors.topMargin: 0.1 * dpi
        spacing: 0.1 * dpi

        GridLayout {
            id: gridLayout
            columns: 4
            width: parent.width

            Label {
                id: fromLabel
                text: qsTr("From: ")
            }

            Row {
                Layout.columnSpan: 3
                Layout.minimumWidth: 6 * dpi

                PasswordDialog {
                    id: accountUnlockDialog
                    //standardButtons: StandardButton.Ok | StandardButton.Cancel

                    onAccepted: {
                        accountModel.unlockAccount(password, settings.value("ipc/accounts/lockduration", 300), fromField.currentIndex)
                    }
                }

                ToolButton {
                    id: lockTool
                    iconSource: accountModel.isLocked(fromField.currentIndex) ? "/images/locked" : "/images/unlocked"
                    width: fromField.height
                    height: fromField.height

                    Connections {
                        target: accountModel
                        onAccountLockedChanged: {
                            lockTool.iconSource = accountModel.isLocked(fromField.currentIndex) ? "/images/locked" : "/images/unlocked"
                            transactionWarning.refresh()
                        }
                    }

                    onClicked: {
                        accountModel.selectedAccountRow = fromField.currentIndex
                        accountUnlockDialog.openFocused("Unlock " + accountModel.selectedAccount)
                    }
                }

                ComboBox {
                    id: fromField
                    width: parent.width - lockTool.width
                    model: accountModel
                    textRole: "summary"
                    onCurrentIndexChanged: transactionWarning.refresh()
                }
            }

            Label {
                id: toLabel
                text: qsTr("To: ")
            }

            TextField {
                id: toField
                validator: RegExpValidator {
                    regExp: /0x[a-f,0-9]{40}/
                }

                maximumLength: 42
                Layout.minimumWidth: 6 * dpi
                Layout.columnSpan: 3

                onTextChanged: transactionWarning.refresh()
            }

            Row {
                ToolButton {
                    id: transactionWarning
                    iconSource: "/images/warning"
                    tooltip: qsTr("Sending checks", "before sending a transaction")
                    width: sendButton.height
                    height: sendButton.height

                    function check() {
                        var result = {
                            error: null,
                            from: null,
                            to: null,
                            value: -1
                        }

                        if ( fromField.currentIndex < 0 ) {
                            result.error = qsTr("Sender account not selected")
                            return result
                        }
                        var index = fromField.currentIndex
                        result.from = accountModel.getAccountHash(index) || ""

                        if ( !result.from.match(/0x[a-f,0-9]{40}/) ) {
                            result.error = qsTr("Sender account invalid")
                            return result
                        }

                        if ( accountModel.isLocked(index) ) {
                            result.error = qsTr("From account is locked")
                            return result
                        }

                        result.to = toField.text || ""
                        if ( !result.to.match(/0x[a-f,0-9]{40}/) ) {
                            result.error = qsTr("Recipient account invalid")
                            return result
                        }

                        result.txtVal = valueField.text.trim() || ""
                        result.value = result.txtVal.length > 0 ? Number(result.txtVal) : NaN
                        if ( isNaN(result.value) || result.value <= 0.0 ) {
                            result.error = qsTr("Invalid value")
                            return result
                        }

                        result.txtGas = gasField.text

                        return result;
                    }

                    function refresh() {
                        var result = check()
                        if ( result.error !== null ) {
                            tooltip = result.error
                        }

                        enabled = (result.error !== null)
                    }

                    onClicked: {
                        refresh()

                        if ( enabled ) {
                            errorDialog.msg = tooltip
                            errorDialog.open()
                        }
                    }
                }

                ConfirmDialog {
                    id: confirmDialog
                    msg: qsTr("Confirm transaction send?")

                    onYes: {
                        var result = transactionWarning.check()
                        if ( result.error !== null ) {
                            errorDialog.msg = result.error
                            errorDialog.open()
                            return
                        }

                        transactionModel.sendTransaction(result.from, result.to, result.txtVal, result.txtGas)
                    }
                }

                Button {
                    id: sendButton
                    text: "Send"

                    onClicked: {
                        var result = transactionWarning.check()
                        if ( result.error !== null ) {
                            errorDialog.msg = result.error
                            errorDialog.open()
                            return
                        }

                        confirmDialog.open()
                    }
                }
            }

            Row {
                Layout.columnSpan: 1

                Label {
                    text: qsTr("Value: ")
                }

                TextField {
                    id: valueField
                    validator: DoubleValidator {
                        bottom: 0.000000000000000001 // should be 1 wei
                        decimals: 18
                        locale: "en_US"
                    }

                    maximumLength: 50
                    width: 2 * dpi
                    onTextChanged: transactionWarning.refresh()
                }

                ToolButton {
                    iconSource: "/images/all"
                    width: sendButton.height
                    height: sendButton.height
                    tooltip: qsTr("Send all", "send all ether from account")
                    onClicked: {
                        valueField.text = transactionModel.getMaxValue(fromField.currentIndex, gasField.text)
                    }
                }
            }

            // -- estimate is broken in geth 1.0.1- must wait for later release
            Row {
                Layout.columnSpan: 2
                Layout.minimumWidth: 4.5 * dpi

                Label {
                    text: qsTr("Gas: ")
                }

                TextField {
                    id: gasField
                    width: 80
                    text: settings.value("gas", "90000")
                    validator: IntValidator {
                        bottom: 0
                        locale: "en_US"
                    }

                    onTextChanged: {
                        settings.setValue("gas", text)
                    }
                }

                Label {
                    text: qsTr("Total: ")
                }

                TextField {
                    id: valueTotalField
                    readOnly: true
                    maximumLength: 50
                    width: 2 * dpi
                    validator: DoubleValidator {
                        bottom: 0.000000000000000001 // should be 1 wei
                        decimals: 18
                        locale: "en_US"
                    }

                    text: transactionModel.estimateTotal(valueField.text, gasField.text)
                }
            }

        }

        TransactionDetails {
            id: details
        }

        TableView {
            id: transactionView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - gridLayout.height - parent.spacing

            TableViewColumn {
                horizontalAlignment: Text.AlignRight
                role: "blocknumber"
                title: qsTr("Block#")
                width: 0.75 * dpi
            }
            TableViewColumn {
                role: "senderalias"
                title: qsTr("Sender")
                width: 2.25 * dpi
            }
            TableViewColumn {
                role: "receiveralias"
                title: qsTr("Receiver")
                width: 2.25 * dpi
            }
            TableViewColumn {
                horizontalAlignment: Text.AlignRight
                role: "value"
                title: qsTr("Value (Ether)")
                width: 1.4 * dpi
            }
            TableViewColumn {
                horizontalAlignment: Text.AlignRight
                role: "depth"
                title: qsTr("Depth")
                width: 0.75 * dpi
            }
            model: transactionModel

            Menu {
                id: rowMenu

                MenuItem {
                    text: qsTr("Details")
                    onTriggered: {
                        details.open(transactionModel.getJson(transactionView.currentRow, true))
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
                            if ( transactionView.currentRow >= 0 ) {
                                rowMenu.popup()
                            }
                        }
                    }
                }
            }
        }

    }
}
