import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.3 as D
import QtQuick.Layouts 1.12
import QtQuick.Extras 1.4

Loader {
    property bool hideTrezor: false
    property bool thinClient: ipc.thinClient

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0.2 * dpi
        // currentIndex: settingsBar.currentIndex

        GroupBox {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 0.5 * dpi

            title: qsTr("Basics")

            Row {
                spacing: 0.5 * dpi
                anchors.margins: 0.2 * dpi

                CheckBox {
                    id: clientModeButton
                    text: qsTr("Full node mode")
                    checked: !thinClient

                    onClicked: {
                        thinClient = !clientModeButton.checked
                        settings.setValue("geth/thinclient", !clientModeButton.checked)

                        if ( !clientModeButton.checked ) {
                            settings.setValue("geth/testnet", false)
                        }

                        if ( settings.contains("program/v2firstrun") ) {
                            badge.show(qsTr("Changing node type requires a restart of Etherwall."))
                        }
                    }
                }

                Row {
                    Label {
                        anchors.verticalCenter: parent.verticalCenter
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

        GroupBox {
            id: gethItem
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 0.5 * dpi

            title: qsTr("Geth")

            Column {
                anchors.margins: 0.2 * dpi
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 0.1 * dpi

                Item {
                    id: rowGethDatadir
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: gethDDField.height

                    Label {
                        id: gethDDLabel
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Geth Data Directory: "
                    }

                    TextField {
                        id: gethDDField
                        anchors.left: gethDDLabel.right
                        anchors.right: gethDDButton.left
                        text: settings.value("geth/datadir", "")
                        onTextChanged: {
                            settings.setValue("geth/datadir", gethDDField.text)
                        }
                    }

                    Button {
                        id: gethDDButton
                        anchors.right: parent.right
                        text: qsTr("Choose")

                        onClicked: {
                            ddFileDialog.open()
                        }
                    }

                    D.FileDialog {
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

                Item {
                    id: rowGethPath
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: gethPathField.height

                    Label {
                        id: gethPathLabel
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Geth path: "
                    }

                    TextField {
                        id: gethPathField
                        anchors.left: gethPathLabel.right
                        anchors.right: gethPathButton.left
                        text: settings.value("geth/path", "")
                        onTextChanged: {
                            settings.setValue("geth/path", gethPathField.text)
                        }
                    }

                    Button {
                        id: gethPathButton
                        anchors.right: parent.right
                        text: qsTr("Choose")

                        onClicked: {
                            gethFileDialog.open()
                        }
                    }

                    D.FileDialog {
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

                Item {
                    id: rowGethArgs
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: gethArgsField.height

                    Label {
                        id: gethArgsLabel
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Additional Geth args: "
                    }

                    TextField {
                        id: gethArgsField
                        anchors.left: gethArgsLabel.right
                        anchors.right: parent.right
                        text: settings.value("geth/args", "--syncmode=fast --cache 512")
                        onTextChanged: {
                            settings.setValue("geth/args", gethArgsField.text)
                        }
                    }
                }
            }
        }

        GroupBox {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 0.5 * dpi

            title: qsTr("Node")

            Column {
                anchors.margins: 0.2 * dpi
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 0.1 * dpi

                Label {
                    visible: ipc.thinClient
                    text: qsTr("Node settings only available in full node mode")
                }

                Dialog {
                    id: confirmDialogTestnet
                    title: qsTr("Warning")
                    Text {
                        text: qsTr("Changing the chain requires a restart of Etherwall (and geth if running externally).")
                    }
                }

                Row {
                    enabled: !thinClient
                    width: parent.width

                    Label {
                        anchors.verticalCenter: parent.verticalCenter
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

                    Label {
                        id: logBlocksLabel
                        anchors.verticalCenter: parent.verticalCenter
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


        GroupBox {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 0.5 * dpi
            visible: !hideTrezor

            title: qsTr("TREZOR")

            Row {
                anchors.margins: 0.2 * dpi
                spacing: 0.1 * dpi

                Button {
                    enabled: trezor.initialized
                    text: qsTr("Import accounts")
                    onClicked: trezorImportDialog.display(qsTr("Import addresses from TREZOR?"))
                }

                Dialog {
                    id: accountRemoveDialog
                    title: qsTr("Confirm removal of all TREZOR accounts")
                    standardButtons: Dialog.Yes | Dialog.No | Dialog.Help
                    Text {
                        text: qsTr("All your TREZOR accounts will be removed from Etherwall?")
                    }

                    onHelpRequested: Qt.openUrlExternally("https://www.etherwall.com/faq/#removeaccount")
                    onAccepted: accountModel.removeAccounts()
                }

                Button {
                    text: qsTr("Clear TREZOR accounts")
                    onClicked: accountRemoveDialog.open()
                }
            }
        }
    }

}
