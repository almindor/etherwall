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
    title: qsTr("Transactions")

    //onActiveChanged:

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        GridLayout {
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
                    visible: (transactionInvalid() !== null)

                    function check() {
                        if ( fromField.currentIndex < 0 ) {
                            return qsTr("Sender account not selected")
                        }
                        var index = fromField.currentIndex
                        var account = accountModel.getAccountHash(index)

                        if ( !account.match(/0x[a-f,0-9]{40}/) ) {
                            return qsTr("Sender account invalid")
                        }

                        if ( !toField.text.match(/0x[a-f,0-9]{40}/) ) {
                            return qsTr("Recipient account invalid")
                        }

                        var txt = valueField.text.trim()
                        var val = txt.length > 0 ? Number(txt) : NaN
                        if ( isNaN(val) || val === 0.0 ) {
                            return qsTr("Invalid value")
                        }

                        return null;
                    }

                    function refresh() {
                        var result = check()
                        if ( result !== null ) {
                            tooltip = result
                        }

                        enabled = (result !== null)
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
                        var error = transactionWarning.check()
                        if ( error !== null ) {
                            errorDialog.error = error
                            errorDialog.open()
                            return
                        }

                        transactionModel.sendTransaction(account, toField.text, val)
                    }
                }
            }
        }
    }
}
