import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Extras 1.4

TabView {
    property bool hideTrezor: false
    property bool thinClient: ipc.thinClient

    Tab {
        title: qsTr("Basic")

        Row {
            spacing: 0.5 * dpi
            anchors.margins: 0.2 * dpi
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: parent.left
            }

            // TODO: rename to infodialog
            ErrorDialog {
                id: confirmThinClientDialog
                title: qsTr("Warning")
                msg: qsTr("Changing node type requires a restart of Etherwall.")
            }

            ToggleButton {
                id: clientModeButton
                width: 1 * dpi
                text: qsTr("Thin client")
                checked: thinClient

                onClicked: {
                    thinClient = clientModeButton.checked
                    settings.setValue("geth/thinclient", clientModeButton.checked)

                    if ( clientModeButton.checked ) {
                        settings.setValue("geth/testnet", false)
                    }

                    if ( settings.contains("program/v2firstrun") ) {
                        confirmThinClientDialog.show()
                    }
                }
            }

            Column {
                spacing: 0.1 * dpi
                width: 5 * dpi
                height: 3 * dpi


                Row {
                    width: parent.width

                    Label {
                        text: qsTr("Helper currency: ")
                    }

                    ComboBox {
                        id: defaultFiatCombo
                        width: 1 * dpi
                        model: currencyModel
                        textRole: "name"
                        currentIndex: currencyModel.helperIndex

                        onActivated: currencyModel.setHelperIndex(index)
                    }
                }
            }
        }
    }

    Tab {
        title: qsTr("Geth")

        Column {
            anchors.margins: 0.2 * dpi
            anchors.fill: parent
            spacing: 0.1 * dpi

            Row {
                id: rowGethDatadir
                width: parent.width

                Label {
                    id: gethDDLabel
                    text: "Geth Data Directory: "
                }

                TextField {
                    id: gethDDField
                    width: parent.width - gethDDButton.width - gethDDLabel.width
                    text: settings.value("geth/datadir", "")
                    onTextChanged: {
                        settings.setValue("geth/datadir", gethDDField.text)
                    }
                }

                Button {
                    id: gethDDButton
                    text: qsTr("Choose")

                    onClicked: {
                        ddFileDialog.open()
                    }
                }

                FileDialog {
                    id: ddFileDialog
                    title: qsTr("Geth data directory")
                    selectFolder: true
                    selectExisting: true
                    selectMultiple: false

                    onAccepted: {
                        gethDDField.text = helpers.localURLToString(ddFileDialog.fileUrl)
                    }
                }
            }

            Row {
                id: rowGethPath
                width: parent.width

                Label {
                    id: gethPathLabel
                    text: "Geth path: "
                }

                TextField {
                    id: gethPathField
                    width: parent.width - gethPathLabel.width - gethPathButton.width
                    text: settings.value("geth/path", "")
                    onTextChanged: {
                        settings.setValue("geth/path", gethPathField.text)
                    }
                }

                Button {
                    id: gethPathButton
                    text: qsTr("Choose")

                    onClicked: {
                        gethFileDialog.open()
                    }
                }

                FileDialog {
                    id: gethFileDialog
                    title: qsTr("Geth executable")
                    selectFolder: false
                    selectExisting: true
                    selectMultiple: false

                    onAccepted: {
                        gethPathField.text = helpers.localURLToString(gethFileDialog.fileUrl)
                    }
                }
            }

            // TODO: rename to infodialog
            ErrorDialog {
                id: confirmDialogTestnet
                title: qsTr("Warning")
                msg: qsTr("Changing the chain requires a restart of Etherwall (and geth if running externally).")
            }

            Row {
                id: rowGethArgs
                width: parent.width

                Label {
                    id: gethArgsLabel
                    text: "Additional Geth args: "
                }

                TextField {
                    id: gethArgsField
                    width: parent.width - gethArgsLabel.width
                    text: settings.value("geth/args", "--syncmode=fast --cache 512")
                    onTextChanged: {
                        settings.setValue("geth/args", gethArgsField.text)
                    }
                }
            }

            Row {
                enabled: !thinClient
                id: rowGethTestnet
                width: parent.width

                Label {
                    id: gethTestnetLabel
                    text: "Testnet (rinkeby): "
                }

                CheckBox {
                    id: gethTestnetCheck
                    checked: settings.valueBool("geth/testnet", false)
                    onClicked: {
                        settings.setValue("geth/testnet", gethTestnetCheck.checked)
                        if ( settings.contains("program/v2firstrun") ) {
                            confirmDialogTestnet.show()
                        }
                    }
                }
            }
        }
    }

    Tab {
        title: qsTr("Advanced")
        enabled: !ipc.thinClient

        Column {
            anchors.margins: 0.2 * dpi
            anchors.fill: parent
            spacing: 0.1 * dpi


            Row {
                enabled: !thinClient
                width: parent.width

                Label {
                    text: qsTr("Update interval (s): ")
                }

                SpinBox {
                    id: intervalSpinBox
                    width: 1 * dpi
                    minimumValue: 5
                    maximumValue: 60

                    value: settings.value("ipc/interval", 10)
                    onValueChanged: ipc.setInterval(intervalSpinBox.value * 1000)
                }
            }

            Row {
                id: rowLogBlocks
                enabled: !thinClient
                width: parent.width

                Label {
                    id: logBlocksLabel
                    text: qsTr("Event history in blocks: ")
                }

                SpinBox {
                    id: logBlocksField
                    width: 1 * dpi
                    minimumValue: 0
                    maximumValue: 100000
                    value: settings.value("geth/logsize", 7200)
                    onValueChanged: {
                        settings.setValue("geth/logsize", logBlocksField.value)
                        filterModel.loadLogs()
                    }
                }
            }
        }
    }


    Tab {
        enabled: !hideTrezor
        title: "TREZOR"

        Column {
            anchors.margins: 0.2 * dpi
            anchors.fill: parent
            spacing: 0.1 * dpi

            Row {
                width: parent.width
                spacing: 0.05 * dpi

                Label {
                    text: qsTr("Import addresses: ")
                }

                SpinBox {
                    id: addressesSpinBox
                    width: 1 * dpi
                    minimumValue: 1
                    maximumValue: 60

                    value: settings.value("trezor/addresses", 5)
                    onValueChanged: settings.setValue("trezor/addresses", addressesSpinBox.value)
                }

                Button {
                    enabled: trezor.initialized
                    text: qsTr("Import")
                    onClicked: accountModel.trezorImport()
                }
            }

            Row {
                spacing: 0.05 * dpi

                ConfirmDialog {
                    id: accountRemoveDialog
                    title: qsTr("Confirm removal of all TREZOR accounts")
                    msg: qsTr("All your TREZOR accounts will be removed from Etherwall") + ' <a href="http://www.etherwall.com/faq/#removeaccount">?</a>'
                    onYes: accountModel.removeAccounts()
                }

                Label {
                    text: qsTr("Clear TREZOR accounts")
                }

                Button {
                    text: qsTr("Clear")
                    onClicked: accountRemoveDialog.open()
                }
            }
        }
    }


}
