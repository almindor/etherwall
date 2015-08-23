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
    enabled: !ipc.busy && (ipc.connectionState > 0)
    title: qsTr("Transactions")

    //onActiveChanged:

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

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
                Layout.minimumWidth: 600

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
                    width: 24
                    height: 24

                    Connections {
                        target: accountModel
                        onAccountUnlocked: {
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
                Layout.minimumWidth: 600
                Layout.columnSpan: 3

                onTextChanged: transactionWarning.refresh()
            }

            Row {
                ToolButton {
                    id: transactionWarning
                    iconSource: "/images/warning"
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

                        transactionModel.sendTransaction(result.from, result.to, result.txtVal)
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
                    Layout.minimumWidth: 250
                    onTextChanged: transactionWarning.refresh()
                }
            }

            // -- estimate is broken in geth 1.0.1- must wait for later release
            Row {
                Layout.columnSpan: 2
                Layout.minimumWidth: 450

                Button {
                    text: qsTr("Estimate total")
                    onClicked: {
                        var result = transactionWarning.check()
                        if ( result.error !== null ) {
                            errorDialog.msg = result.error
                            errorDialog.open()
                            return
                        }

                        ipc.estimateGas(result.from, result.to, result.value)
                    }
                }

                TextField {
                    id: valueTotalField
                    readOnly: true
                    maximumLength: 50
                    width: 300
                    validator: DoubleValidator {
                        bottom: 0.000000000000000001 // should be 1 wei
                        decimals: 18
                    }

                    text: Number(valueField.text) > 0 ? Number(valueField.text) + Number(transactionModel.gasPrice) * Number(transactionModel.gasEstimate) : ""
                }
            }

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
                width: 70
            }
            TableViewColumn {
                role: "sender"
                title: qsTr("Sender")
                width: 200
            }
            TableViewColumn {
                role: "receiver"
                title: qsTr("Receiver")
                width: 200
            }
            TableViewColumn {
                horizontalAlignment: Text.AlignRight
                role: "value"
                title: qsTr("Value (Ether)")
                width: 150
            }
            TableViewColumn {
                horizontalAlignment: Text.AlignRight
                role: "depth"
                title: qsTr("Depth")
                width: 70
            }
            model: transactionModel
        }

    }
}
