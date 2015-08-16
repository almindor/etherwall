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
import QtQuick.Dialogs 1.2
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
            columns: 3
            width: parent.width

            Label {
                id: fromLabel
                text: qsTr("From: ")
            }

            ComboBox {
                id: fromField
                Layout.minimumWidth: 600
                Layout.columnSpan: 2
                model: accountModel
                textRole: "summary"
                onCurrentIndexChanged: transactionWarning.refresh()
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
                Layout.columnSpan: 2

                onTextChanged: transactionWarning.refresh()
            }

            Label {
                text: qsTr("Value [Ether]: ")
            }

            TextField {
                id: valueField
                validator: DoubleValidator {
                    bottom: 0.000000000000000001 // should be 1 wei
                    decimals: 18
                }

                maximumLength: 50
                Layout.minimumWidth: 100

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

                        result.to = toField.text || ""
                        if ( !result.to.match(/0x[a-f,0-9]{40}/) ) {
                            result.error = qsTr("Recipient account invalid")
                            return result
                        }

                        var txt = valueField.text.trim() || ""
                        result.value = txt.length > 0 ? Number(txt) : NaN
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
                            errorDialog.error = tooltip
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
                            errorDialog.error = result.error
                            errorDialog.open()
                            return
                        }

                        transactionModel.sendTransaction(result.from, result.to, result.value)
                    }
                }
            }
        }


        TableView {
            id: transactionView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - gridLayout.height - parent.spacing

            TableViewColumn {
                role: "hash"
                title: qsTr("Hash")
                width: 150
            }
            TableViewColumn {
                role: "sender"
                title: qsTr("Sender")
                width: 150
            }
            TableViewColumn {
                role: "receiver"
                title: qsTr("Receiver")
                width: 150
            }
            TableViewColumn {
                role: "value"
                title: qsTr("Value (Ether)")
            }
            model: transactionModel
        }

    }
}
