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
/** @file FirstTimeDialog.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * FirstTime dialog
 */

import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Window 2.0

Window {
    id: sendDialog
    title: qsTr("Send Ether")

    modality: Qt.NonModal
    visible: false
    minimumWidth: 6 * dpi
    minimumHeight: 1 * dpi
    maximumWidth: 10 * dpi
    maximumHeight: 8 * dpi
    width: 7 * dpi
    height: 5 * dpi
    Component.onCompleted: {
        setX(Screen.width / 2.0 - width / 2.0)
        setY(Screen.height / 2.0 - height / 2.0)
    }

    property bool done: false

    BusyIndicator {
        anchors.centerIn: parent
        z: 10
        running: ipc.starting || ipc.busy || ipc.syncing
    }

    Column {
        id: mainColumn
        anchors.fill: parent
        anchors.margins: 0.1 * dpi
        spacing: 0.2 * dpi

        Row {
            Label {
                width: 1 * dpi
                text: qsTr("From: ")
            }

            ComboBox {
                id: fromField
                width: mainColumn.width - 1 * dpi
                model: accountModel
                textRole: "summary"
                onCurrentIndexChanged: {
                    sendButton.refresh()
                }
            }
        }

        Row {
            Label {
                width: 1 * dpi
                text: qsTr("To: ")
            }

            TextField {
                id: toField
                width: mainColumn.width - 1 * dpi
                validator: RegExpValidator {
                    regExp: /0x[a-f,0-9]{40}/
                }

                maximumLength: 42

                onTextChanged: sendButton.refresh()
            }
        }

        Row {
            Label {
                text: qsTr("Gas: ")
                width: 1 * dpi
            }

            ComboBox {
                id: gasCombo
                width: 1.2 * dpi
                currentIndex: {
                    var gas = parseInt(settings.value("gas", 21000))
                    for ( var i = 0; i < 4; i++ ) {
                        if ( gasItems.get(i).value === gas ) {
                            return i;
                        }
                    }

                    return 3
                }

                model: ListModel {
                    id: gasItems
                    ListElement { text: "Default"; value: 90000 }
                    ListElement { text: "Minimal"; value: 21000 }
                    ListElement { text: "Fast"; value: 180000 }
                    ListElement { text: "Custom"; value: 0 }
                }

                onCurrentIndexChanged: {
                    // custom
                    if ( currentIndex == 3 ) {
                        gasField.text = settings.value("gas", 21000)
                        gasField.readOnly = false
                        return
                    }

                    if ( currentIndex < 0 || currentIndex > 3 ) {
                        return // ???
                    }

                    var gas = gasItems.get(currentIndex).value
                    gasField.text = gas
                    gasField.readOnly = true
                }
            }

            TextField {
                id: gasField
                readOnly: true
                width: 0.9 * dpi
                validator: IntValidator {
                    bottom: 21000
                    top: 99999999
                }

                onTextChanged: {
                    settings.setValue("gas", text)
                }
            }

            Label {
                width: 1 * dpi
                text: " " + qsTr("Gas Price: ")
            }

            TextField {
                id: gasPriceField
                width: mainColumn.width - gasField.width - gasCombo.width - 2 * dpi - gasButton.width
                text: transactionModel.gasPrice
                validator: DoubleValidator {
                    bottom: 0.000000000000000001 // should be 1 wei
                    decimals: 18
                    locale: "en_US"
                }

                onEditingFinished: {
                    // prevent updates from now on until they press gasPrice refresh if value is new
                    if ( text !== transactionModel.gasPrice ) {
                        text = String(text)
                    }
                }
            }

            ToolButton {
                id: gasButton
                height: 32
                width: 32
                iconSource: "/images/gas"
                tooltip: qsTr("Apply current gas price")
                onClicked: {
                    gasPriceField.text = Qt.binding(function() { return transactionModel.gasPrice })
                }
            }
        }

        Row {
            Label {
                width: 1 * dpi
                text: qsTr("Value: ")
            }

            TextField {
                id: valueField
                width: mainColumn.width - 1 * dpi - sendAllButton.width
                validator: DoubleValidator {
                    bottom: 0.000000000000000001 // should be 1 wei
                    decimals: 18
                    locale: "en_US"
                }

                maximumLength: 50
                onTextChanged: sendButton.refresh()
            }

            ToolButton {
                id: sendAllButton
                iconSource: "/images/all"
                width: 32
                height: 32

                tooltip: qsTr("Send all", "send all ether from account")
                onClicked: {
                    valueField.text = transactionModel.getMaxValue(fromField.currentIndex, gasField.text, gasPriceField.text)
                }
            }
        }

        // -- estimate is broken in geth 1.0.1- must wait for later release
        Row {
            id: lastRow
            Label {
                width: 1 * dpi
                text: qsTr("Total: ")
            }

            TextField {
                id: valueTotalField
                readOnly: true
                maximumLength: 50
                width: mainColumn.width - 1 * dpi
                validator: DoubleValidator {
                    bottom: 0.000000000000000001 // should be 1 wei
                    decimals: 18
                    locale: "en_US"
                }

                text: transactionModel.estimateTotal(valueField.text, gasField.text)
            }
        }

        PasswordDialog {
            id: transactionSendDialog
            title: qsTr("Confirm transaction")

            onAccepted: {
                var result = sendButton.check()
                if ( result.error !== null ) {
                    errorDialog.msg = result.error
                    errorDialog.open()
                    return
                }

                transactionModel.sendTransaction(password, result.from, result.to, result.txtVal, result.txtGas, result.txtGasPrice)
            }
        }

        ErrorDialog {
            id: confirmDialog
            title: qsTr("Transaction sent")
            msg: qsTr("Transaction has been successfully sent and is now pending. Check the transactions tab for more info.")
        }

        Button {
            id: sendButton
            enabled: !ipc.syncing && !ipc.closing && !ipc.starting
            width: parent.width
            height: 1.3 * dpi
            text: "Send"

            Image {
                id: sendIcon
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: parent.height * 0.15
                width: height
                source: "/images/warning"
            }

            style: ButtonStyle {
              label: Text {
                renderType: Text.NativeRendering
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: sendButton.height / 2.0
                text: control.text
              }
            }

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

                result.txtVal = valueField.text.trim() || ""
                result.value = result.txtVal.length > 0 ? Number(result.txtVal) : NaN
                if ( isNaN(result.value) || result.value <= 0.0 ) {
                    result.error = qsTr("Invalid value")
                    return result
                }

                result.txtGas = gasField.text
                result.txtGasPrice = gasPriceField.text

                return result;
            }

            function refresh() {
                var result = check()
                if ( result.error !== null ) {
                    tooltip = result.error
                    sendIcon.source = "/images/warning"
                    return result
                }

                sendIcon.source = "/images/ok"
                return result
            }

            Connections {
                target: ipc
                onError: {
                    sendButton.enabled = true
                }

                onSendTransactionDone: {
                    confirmDialog.show()
                }
            }

            onClicked: {
                var result = refresh()
                if ( result.error !== null ) {
                    errorDialog.msg = result.error
                    errorDialog.open()
                    return
                }

                transactionSendDialog.msg = qsTr("Confirm send of Ξ") + result.value + qsTr(" to: ") + result.to
                transactionSendDialog.open()
            }
        }

        Button {
            text: qsTr("Close")
            width: parent.width
            height: 0.6 * dpi

            onClicked: {
                sendDialog.close()
            }
        }

    }
}
