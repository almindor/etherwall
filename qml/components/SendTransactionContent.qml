import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

Item {
    id: transactionContent
    anchors.fill: parent
    property int decimals : 18
    property string tokenAddress: ""
    property string toAddress : ""
    property string contractData : ""
    property string txValue : "0"
    property bool functionIsConstant : false
    property int callIndex : -1
    property var userData : null
    // deployment properties
    property string contractName : ""
    property string contractAbi : ""
    signal done

    function prepare() {
        valueField.text = "0"
        toField.text = toAddress
        tokenCombo.currentIndex = (toAddress.length === 0 && contractName.length === 0) ? tokenModel.outerIndex : 0
        tokenAddress = tokenModel.getTokenAddress(tokenCombo.currentIndex)

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

    Timer {
        id: valueChangedTimer
        interval: 250
        repeat: false
        running: false
        onTriggered: {
            var result = sendButton.refresh()
            if ( !result.error ) {
                ipc.estimateGas(result.from, result.to, result.txtVal, "3141592", result.txtGasPrice, contractData)
            }
        }
    }

    Connections {
        id: ipcConnection
        property string deployedName
        property string deployedAbi
        property string txTo
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
            if ( ipcConnection.deployedName.length ) {
                contractModel.addPendingContract(ipcConnection.deployedName, ipcConnection.deployedAbi, hash)
                badge.show(qsTr("New pending contract deployment: ") + ipcConnection.deployedName)
            } else {
                badge.show(qsTr("New pending transaction to: ") + toField.text)
            }

            contractName = ""
            contractAbi = "[]"
            contractData = ""
            done()
        }
    }

    Connections {
        target: trezor
        onPresenceChanged: sendButton.refresh()
    }

    Connections {
        target: contractModel

        onCallEncoded: {
            var result = sendButton.refresh()
            if ( !result.error ) {
                ipc.estimateGas(result.from, result.to, result.txtVal, "3141592", result.txtGasPrice, contractData)
            }
        }
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
                validator: RegExpValidator {
                    regExp: /^[0-9]+([.][0-9]{1,18})?$/
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
                width: mainColumn.width - 1 * dpi - sendAllButton.width - tokenCombo.width
                validator: RegExpValidator {
                    id: valueValidator
                    regExp: /^[0-9]{1,40}([.][0-9]{1,18})?$/
                }

                maximumLength: 50
                onTextChanged: {
                    if ( sendButton ) {
                        txValue = text
                        if ( tokenCombo.currentIndex > 0 ) {
                            txValue = "0" // enforce 0 ETH on token sends
                        }
                        var result = sendButton.refresh()
                        contractData = tokenModel.getTokenTransferData(tokenCombo.currentIndex, toField.text, text)
                        if ( !result.error ) {
                            valueChangedTimer.restart()
                        }
                    }
                }
                text: "0"
            }

            ComboBox {
                id: tokenCombo
                width: 1 * dpi
                enabled: (toAddress.length === 0 && contractName.length === 0) // not an invoke or deploy
                model: tokenModel
                textRole: "token"
                onActivated: {
                    tokenModel.selectToken(index)
                    tokenAddress = tokenModel.getTokenAddress(index)
                    decimals = tokenModel.getTokenDecimals(index)

                    txValue = "0"
                    if ( valueField.text !== "0" ) {
                        valueField.text = 0
                    } else {
                        var result = sendButton.refresh()
                        contractData = tokenModel.getTokenTransferData(index, toField.text, "0")
                        ipc.estimateGas(result.from, result.to, result.txtVal, "3141592", result.txtGasPrice, contractData)
                    }
                    var regstr = "^[0-9]{1,40}([.][0-9]{1," + decimals + "})?$"
                    valueValidator.regExp = new RegExp(regstr)
                }
            }

            ToolButton {
                id: sendAllButton
                iconSource: "/images/all"
                width: 32
                height: 32

                tooltip: qsTr("Send all", "send all ether from account")
                onClicked: {
                    if ( tokenCombo.currentIndex > 0 ) {
                        valueField.text = accountModel.getMaxTokenValue(fromField.currentIndex, tokenAddress)
                    } else {
                        valueField.text = transactionModel.getMaxValue(fromField.currentIndex, gasField.text, gasPriceField.text)
                    }
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
                validator: RegExpValidator {
                    regExp: /^[0-9]+([.][0-9]{1,18})?$/
                }

                text: transactionModel.estimateTotal(txValue, gasField.text, gasPriceField.text, tokenAddress)
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
                textFormat: Text.RichText
                readOnly: true
                height: 0.5 * dpi
                width: mainColumn.width - 1 * dpi
                onLinkActivated: Qt.openUrlExternally(link)

                ColorAnimation on textColor {
                    id: errorAnimation
                    from: "black"
                    to: "red"
                    duration: 250
                    running: false
                    loops: 5
                    onStopped: warningField.textColor = "black"
                }
            }
        }

        PasswordDialog {
            id: transactionSendDialog
            title: qsTr("Confirm transaction")

            onPasswordSubmitted: {
                var result = sendButton.check(fromField.currentIndex)
                if ( result.error !== null ) {
                    errorDialog.text = result.error
                    errorDialog.open()
                    return
                }

                transactionModel.sendTransaction(password, result.from, result.to, result.txtVal, result.nonce, result.txtGas, result.txtGasPrice, contractData)
            }
        }

        Button {
            id: sendButton
            enabled: !ipc.syncing && !ipc.closing && !ipc.starting && !ipc.busy
            width: parent.width
            height: 1.3 * dpi
            text: status > -2 ? qsTr("Send") : qsTr("Input errors")
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
                    result.error = qsTr('Address checksum mismatch. <a href="https://www.etherwall.com/faq/#checksum">Click here for more info.</a>')
                }

                result.txtVal = txValue.trim() || ""
                result.value = result.txtVal.length > 0 ? Number(result.txtVal) : NaN
                if ( isNaN(result.value) || result.value < 0.0 ) {
                    result.error = qsTr("Invalid value")
                    return result
                }


                result.chain_id = ipc.testnet ? 4 : 1 // TODO: this should be handled better
                result.nonce = accountModel.getAccountNonce(accountIndex)
                result.hdpath = accountModel.getAccountHDPath(accountIndex)

                if ( !functionIsConstant && result.hdpath && !trezor.present ) {
                    result.error = qsTr("Connect TREZOR device to send from external account")
                    return result;
                }

                result.txtGas = gasField.text
                result.txtGasPrice = gasPriceField.text

                // if we're tokening use token address and value
                if ( tokenCombo.currentIndex > 0 ) {
                    result.to = tokenAddress // we're sending to token contract, actual destination is in data
                    result.txtVal = 0 // we're sending 0 eth, token size is in data
                }

                return result;
            }

            function getGasTotal() {
                return parseFloat(gasField.text) * parseFloat(gasPriceField.text)
            }

            function setHelperText(total, gasTotal) {
                if ( status !== 0 ) {
                    return
                }

                if (functionIsConstant) {
                    warningField.text = qsTr("Constant method invocation is free")
                    return
                }

                warningField.text = currencyModel.helperName + " total: " + currencyModel.recalculateToHelper(total) + "<br>" +
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
                    status = 0
                    warningField.text = ""
                    setHelperText(valueTotalField.text, getGasTotal())
                    toField.textColor = "black"
                    sendIcon.source = "/images/ok"
                }

                return result
            }

            onClicked: {
                var result = refresh()
                if ( result.error !== null ) {
                    errorAnimation.start()
                    return
                }

                // if it's a constant functionc all just do eth_call
                if ( functionIsConstant ) {
                    transactionModel.call(result.from, result.to, result.txtVal, result.txtGas, result.txtGasPrice, contractData, callIndex, userData)
                    return
                }

                // otherwise full TX, set expectations
                ipcConnection.deployedName = contractName
                ipcConnection.deployedAbi = contractAbi
                ipcConnection.txTo = result.to

                // trezor asks for confirmation[s] on it's display, one more is cumbersome
                if ( result.hdpath.length ) {
                    trezor.signTransaction(result.chain_id, result.hdpath, result.from, result.to, result.txtVal, result.nonce, result.txtGas, result.txtGasPrice, contractData)
                    return
                }

                if ( contractName.length > 0 ) {
                    transactionSendDialog.text = qsTr("Confirm creation of contract: ") + contractName
                } else if ( tokenCombo.currentIndex === 0 ) {
                    transactionSendDialog.text = qsTr("Confirm send of Ξ") + result.value + qsTr(" to: ") + result.to
                } else {
                    transactionSendDialog.text = qsTr("Confirm send of ") + valueField.text + " " + tokenCombo.currentText + qsTr(" to: ") + toField.text
                }

                transactionSendDialog.open()
            }
        }
    }

}
