import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Dialogs 1.0

TabView {
    Tab {
        title: qsTr("Basic")

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
                        gethDDField.text = ddFileDialog.fileUrl.toString().replace(/^(file:\/{3})/,""); // fugly but gotta love QML on this one
                    }
                }
            }

            Row {
                width: parent.width

                Label {
                    text: qsTr("Account unlock duration (s): ")
                }

                SpinBox {
                    id: unlockDurSpinBox
                    width: 100
                    minimumValue: 10
                    maximumValue: 3600

                    value: settings.value("ipc/accounts/lockduration", 300)
                    onValueChanged: {
                        settings.setValue("ipc/accounts/lockduration", value)
                    }
                }
            }

            Row {
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
                    onValueChanged: {
                        settings.setValue("ipc/interval", intervalSpinBox.value)
                        ipc.setInterval(intervalSpinBox.value * 1000)
                    }
                }
            }

        }
    }

    Tab {
        title: qsTr("Advanced")

        Column {
            anchors.margins: 0.2 * dpi
            anchors.fill: parent
            spacing: 0.1 * dpi

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
                        gethPathField.text = gethFileDialog.fileUrl.toString().replace(/^(file:\/{3})/,""); // fugly but gotta love QML on this one
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
                    width: parent.width - gethArgsLabel.width
                    text: settings.value("geth/args", "--fast --cache 512")
                    onTextChanged: {
                        settings.setValue("geth/args", gethArgsField.text)
                    }
                }
            }

        }
    }

}
