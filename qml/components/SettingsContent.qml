import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2
import QtQuick.Extras 1.4

Loader {
    property bool hideTrezor: false
    property bool thinClient: ipc.thinClient

    TabBar {
        id: settingsBar
        anchors.left: parent.left
        anchors.right: parent.right

        TabButton {
            text: qsTr("Basic")
        }
        TabButton {
            text: qsTr("Geth")
        }
        TabButton {
            text: qsTr("Advanced")
        }
        TabButton {
            enabled: !hideTrezor
            text: qsTr("TREZOR")
        }
    }

    StackLayout {
        id: gethItem
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: settingsBar.bottom
        anchors.bottom: parent.bottom

        currentIndex: settingsBar.currentIndex

        Item {
            Row {
                spacing: 0.5 * dpi
                anchors.margins: 0.2 * dpi
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    left: parent.left
                }

                MessageDialog {
                    id: confirmThinClientDialog
                    icon: StandardIcon.Warning
                    title: qsTr("Warning")
                    text: qsTr("Changing node type requires a restart of Etherwall.")
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
                            confirmThinClientDialog.open()
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

        Item {
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
                        width: gethItem.width - gethDDButton.width - gethDDLabel.width - 0.2 * dpi
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
                        width: gethItem.width - gethPathLabel.width - gethPathButton.width - 0.2 * dpi
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

                Row {
                    id: rowGethArgs
                    width: parent.width

                    Label {
                        id: gethArgsLabel
                        text: "Additional Geth args: "
                    }

                    TextField {
                        id: gethArgsField
                        width: gethItem.width - gethArgsLabel.width - 0.2 * dpi
                        text: settings.value("geth/args", "--syncmode=fast --cache 512")
                        onTextChanged: {
                            settings.setValue("geth/args", gethArgsField.text)
                        }
                    }
                }
            }
        }

        Item {
            Column {
                anchors.margins: 0.2 * dpi
                anchors.fill: parent
                spacing: 0.1 * dpi

                Label {
                    visible: ipc.thinClient
                    text: qsTr("Advanced settings only available in full node mode")
                }

                MessageDialog {
                    id: confirmDialogTestnet
                    icon: StandardIcon.Warning
                    title: qsTr("Warning")
                    text: qsTr("Changing the chain requires a restart of Etherwall (and geth if running externally).")
                }

                Row {
                    enabled: !thinClient
                    width: parent.width

                    Label {
                        text: qsTr("Update interval (s): ")
                    }

                    SpinBox {
                        id: intervalSpinBox
                        width: 1 * dpi
                        validator: IntValidator {
                            bottom: 5
                            top: 60
                        }

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
                        validator: IntValidator {
                            bottom: 0
                            top: 100000
                        }
                        value: settings.value("geth/logsize", 7200)
                        onValueChanged: {
                            settings.setValue("geth/logsize", logBlocksField.value)
                            filterModel.loadLogs()
                        }
                    }
                }

                CheckBox {
                    id: gethTestnetCheck
                    enabled: !thinClient
                    checked: settings.valueBool("geth/testnet", false)
                    text: qsTr("Testnet (rinkeby)")
                    onClicked: {
                        settings.setValue("geth/testnet", gethTestnetCheck.checked)
                        if ( settings.contains("program/v2firstrun") ) {
                            confirmDialogTestnet.open()
                        }
                    }
                }
            }
        }


        Item {
            Column {
                anchors.margins: 0.2 * dpi
                anchors.fill: parent
                spacing: 0.1 * dpi

                Button {
                    enabled: trezor.initialized
                    text: qsTr("Import accounts")
                    onClicked: trezorImportDialog.display(qsTr("Import addresses from TREZOR?"))
                }

                Row {
                    spacing: 0.05 * dpi

                    MessageDialog {
                        id: accountRemoveDialog
                        title: qsTr("Confirm removal of all TREZOR accounts")
                        text: qsTr("All your TREZOR accounts will be removed from Etherwall?")
                        standardButtons: StandardButton.Yes | StandardButton.No | StandardButton.Help
                        onHelp: Qt.openUrlExternally("https://www.etherwall.com/faq/#removeaccount")
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

}
