import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

Item {
    id: transactionContent
    anchors.fill: parent
    property string toAddress : ""
    property string contractData : ""
    // deployment properties
    property string contractName : ""
    property string contractAbi : ""
    signal done

    function prepare() {
        if ( contractName.length ) {
            gasField.text = "3141592"
        } else {
            gasField.text = "21000"
        }
        var result = sendButton.refresh()
        if ( !result.error ) {
            ipc.estimateGas(result.from, result.to, result.txtVal, "3141592", result.txtGasPrice, contractData)
        }
    }

    Component.onCompleted: prepare()

    Connections {
        target: trezor
        onPresenceChanged: sendButton.refresh()
    }

    BusyIndicator {
        anchors.centerIn: parent
        z: 10
        running: ipc.starting || ipc.busy || ipc.syncing
    }

    Column {
        id: mainColumn
        anchors.fill: parent
        anchors.margins: 0.1 * dpi
        spacing: 0.1 * dpi

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
                currentIndex: accountModel.defaultIndex
                onActivated: {
                    var result = sendButton.refresh(index)
                    if ( !result.error ) {
                        ipc.estimateGas(result.from, result.to, result.txtVal, "3141592", result.txtGasPrice, contractData)
                    }
                }
            }
        }

        Row {
            visible: contractName.length === 0
            Label {
                width: 1 * dpi
                text: qsTr("To: ")
            }

            TextField {
                id: toField
                width: mainColumn.width - 1 * dpi
                validator: RegExpValidator {
                    regExp: /0x[a-f,A-F,0-9]{40}/
                }

                text: toAddress
                readOnly: contractData.length > 0
                maximumLength: 42

                onTextChanged: {
                    var result = sendButton.refresh()
                    if ( !result.error ) {
                        ipc.estimateGas(result.from, result.to, result.txtVal, "3141592", result.txtGasPrice, contractData)
                    }
                }
            }
        }

        Row {
            Label {
                text: qsTr("Gas: ")
                width: 1 * dpi
            }

            TextField {
                property bool manual : false

                id: gasField
                width: 0.9 * dpi
                validator: IntValidator {
                    bottom: 21000
                    top: 99999999
                }

                text: "3141592" // max, will go lower on estimates

                onEditingFinished: manual = true
            }

            Button {
                id: estimateButton
                text: "Estimate"
                onClicked: {
                    gasField.manual = false // allow overrides
                    var result = sendButton.refresh()
                    if ( !result.error ) {
                        ipc.estimateGas(result.from, result.to, result.txtVal, "3141592", result.txtGasPrice, contractData)
                    }
                }
            }

            Label {
                width: 1 * dpi
                text: " " + qsTr("Gas Price: ")
            }

            TextField {
                id: gasPriceField
                width: mainColumn.width - gasField.width - estimateButton.width - 2 * dpi - gasButton.width
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
                        var result = sendButton.refresh()
                        if ( !result.error ) {
                            ipc.estimateGas(result.from, result.to, result.txtVal, "3141592", result.txtGasPrice, contractData)
                        }
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
                    var result = sendButton.refresh()
                    if ( !result.error ) {
                        ipc.estimateGas(result.from, result.to, result.txtVal, "3141592", result.txtGasPrice, contractData)
                    }
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
                onTextChanged: {
                    if ( !text || !text.length || ( sendButton && sendButton.status < 0 ) ) sendButton.refresh()
                }
                text: "0"
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

                text: transactionModel.estimateTotal(valueField.text, gasField.text, gasPriceField.text)
                onTextChanged: sendButton.setHelperText(text, sendButton.getGasTotal())
            }
        }

        Row {
            id: errorRow

            Label {
                width: 1 * dpi
                text: sendButton.status < -1 ? qsTr("Error: ") : qsTr("Warning: ")
                enabled: sendButton.status < 0
            }

            TextArea {
                id: warningField
                readOnly: true
                height: 0.5 * dpi
                width: mainColumn.width - 1 * dpi
            }
        }

        PasswordDialog {
            id: transactionSendDialog
            title: qsTr("Confirm transaction")

            onAccepted: {
                var result = sendButton.check(fromField.currentIndex)
                if ( result.error !== null ) {
                    errorDialog.msg = result.error
                    errorDialog.open()
                    return
                }

                transactionModel.sendTransaction(password, result.from, result.to, result.txtVal, result.nonce, result.txtGas, result.txtGasPrice, contractData)
            }
        }

        Button {
            id: sendButton
            enabled: !ipc.syncing && !ipc.closing && !ipc.starting
            width: parent.width
            height: 1.3 * dpi
            text: "Send"
            property int status : -2

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

            function check(accountIndex) {
                var result = {
                    error: null,
                    warning: null,
                    from: null,
                    to: null,
                    value: -1
                }

                if ( accountIndex < 0 ) {
                    result.error = qsTr("Sender account not selected")
                    return result
                }
                result.from = accountModel.getAccountHash(accountIndex) || ""

                if ( !result.from.match(/0x[a-f,A-Z,0-9]{40}/) ) {
                    result.error = qsTr("Sender account invalid")
                    return result
                }

                result.to = toField.text || ""
                if ( contractName.length === 0 && !result.to.match(/0x[a-f,A-Z,0-9]{40}/) ) {
                    result.error = qsTr("Recipient account invalid")
                    return result
                }

                if ( result.to.length && !helpers.checkAddress(result.to) ) {
                    result.warning = qsTr("Address checksum mismatch. Use only properly checksumed addresses to prevent wrong sends.")
                }

                result.txtVal = valueField.text.trim() || ""
                result.value = result.txtVal.length > 0 ? Number(result.txtVal) : NaN
                if ( isNaN(result.value) || result.value < 0.0 ) {
                    result.error = qsTr("Invalid value")
                    return result
                }

                result.chain_id = ipc.testnet ? 4 : 1 // TODO: this should be handled better
                result.nonce = accountModel.getAccountNonce(accountIndex)
                result.hdpath = accountModel.getAccountHDPath(accountIndex)

                if ( result.hdpath && !trezor.present ) {
                    result.error = qsTr("Connect TREZOR device to send from external account")
                    return result;
                }

                result.txtGas = gasField.text
                result.txtGasPrice = gasPriceField.text

                return result;
            }

            function getGasTotal() {
                return parseFloat(gasField.text) * parseFloat(gasPriceField.text)
            }

            function setHelperText(total, gasTotal) {
                if ( status !== 0 ) {
                    return
                }

                warningField.text = currencyModel.helperName + " total: " + currencyModel.recalculateToHelper(total) + "\n" +
                        currencyModel.helperName + " gas cost: " + currencyModel.recalculateToHelper(gasTotal)
            }

            function refresh(accountIndex) {
                if ( accountIndex === undefined || accountIndex === null ) {
                    accountIndex = fromField.currentIndex
                }

                var result = check(accountIndex)

                if ( result.error !== null ) {
                    tooltip = result.error
                    sendIcon.source = "/images/error"
                    status = -2
                    warningField.text = result.error
                    return result
                }

                if ( result.warning !== null ) {
                    toField.textColor = "brown"
                    warningField.text = result.warning
                    tooltip = result.warning
                    sendIcon.source = "/images/warning"
                    status = -1
                } else {
                    warningField.text = ""
                    toField.textColor = "black"
                    status = 0
                    sendIcon.source = "/images/ok"
                }

                return result
            }

            Connections {
                target: ipc
                onError: {
                    sendButton.enabled = true
                }

                onEstimateGasDone: {
                    if ( !gasField.manual ) {
                        gasField.text = price
                    }
                }

                onSendTransactionDone: {
                    done()
                    if ( contractName.length > 0 ) {
                        contractModel.addPendingContract(contractName, contractAbi, hash)
                        badge.show(qsTr("New pending contract deployment: ") + contractName)
                    } else {
                        badge.show(qsTr("New pending transaction to: ") + toField.text)
                    }
                }
            }

            onClicked: {
                var result = refresh()
                if ( result.error !== null ) {
                    errorDialog.msg = result.error
                    errorDialog.open()
                    return
                }

                // trezor asks for confirmation[s] on it's display, one more is cumbersome
                if ( result.hdpath.length ) {
                    trezor.signTransaction(result.chain_id, result.hdpath, result.from, result.to, result.txtVal, result.nonce, result.txtGas, result.txtGasPrice, contractData)
                    return
                }

                if ( contractName.length > 0 ) {
                    transactionSendDialog.msg = qsTr("Confirm creation of contract: ") + contractName
                } else {
                    transactionSendDialog.msg = qsTr("Confirm send of Ξ") + result.value + qsTr(" to: ") + result.to
                }
                transactionSendDialog.open()
            }
        }

        Button {
            text: qsTr("Close")
            width: parent.width
            height: 0.6 * dpi

            onClicked: {
                done()
            }
        }

    }

}
