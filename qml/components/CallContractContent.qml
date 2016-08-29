import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

Item {
    anchors.fill: parent

    signal done
    signal contractReady(string encoded, bool next)
    signal contractError
    property int contractIndex : -1

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
                text: qsTr("Contract: ")
            }

            TextField {
                id: nameField
                width: mainColumn.width - 1 * dpi
                text: contractModel.getName(contractIndex)
                readOnly: true
            }
        }

        Row {
            Label {
                width: 1 * dpi
                text: qsTr("Function: ")
            }

            ComboBox {
                id: functionField
                width: mainColumn.width - 1 * dpi
                model: contractModel.getFunctions(contractIndex)

                onCurrentTextChanged: {
                    argsView.params = []
                    argsView.model = contractModel.getArguments(contractIndex, currentText)
                    contractModel.encodeCall(contractIndex, functionField.currentText, argsView.params);
                }
            }
        }

        Row {
            Label {
                width: 1 * dpi
                text: qsTr("Call data: ")
            }

            TextField {
                id: encodedText
                width: mainColumn.width - 1 * dpi
                readOnly: true
            }

            TextField {
                id: errorText
                width: mainColumn.width - 1 * dpi
                readOnly: true
                visible: false

                style: TextFieldStyle {
                    textColor: "black"
                    background: Rectangle {
                        radius: 2
                        border.color: "red"
                        border.width: 1
                    }
                }
            }
        }

        ListView {
            id: argsView
            width: parent.width
            height: 1.5 * dpi
            property variant params : []

            delegate: Row {
                Label {
                    width: 2.5 * dpi
                    text: modelData.name + "\t" + modelData.type
                }

                ComboBox {
                    visible: modelData.type === "bool"
                    width: mainColumn.width - 2.5 * dpi
                    editable: false
                    model: ListModel {
                        ListElement { text: "true" }
                        ListElement { text: "false" }
                    }

                    onCurrentTextChanged: {
                        if ( !visible ) return;
                        argsView.params[index] = currentText
                        contractModel.encodeCall(contractIndex, functionField.currentText, argsView.params);
                    }
                }

                TextField {
                    id: valField
                    visible: modelData.type !== "bool"
                    width: mainColumn.width - 2.5 * dpi
                    placeholderText: modelData.placeholder

                    onTextChanged: {
                        if ( !visible ) return;
                        argsView.params[index] = text
                        contractModel.encodeCall(contractIndex, functionField.currentText, argsView.params);
                    }

                    validator: RegExpValidator {
                        regExp: modelData.valrex
                    }
                }
            }
        }

        Connections {
            target: contractModel
            onCallError: {
                errorText.text = err
                errorText.visible = true
                encodedText.visible = false
                encodedText.text = ''
                contractError()
            }

            onCallEncoded: {
                encodedText.text = encoded
                errorText.text = ''
                errorText.visible = false
                encodedText.visible = true
                contractReady(encoded, false)
            }
        }

        Button {
            id: callButton
            width: parent.width
            height: 0.6 * dpi
            text: errorText.text.length ? qsTr("Invalid Input") : qsTr("Setup Transaction")

            Image {
                id: callIcon
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: parent.height * 0.15
                width: height
                source: errorText.text.length ? "/images/warning" : "/images/ok"
            }

            style: ButtonStyle {
              label: Text {
                renderType: Text.NativeRendering
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: callButton.height / 2.0
                text: control.text
              }
            }

            function check() {
                var result = {
                    error: errorText.text.length ? errorText.text : null
                }

                if ( functionField.currentIndex < 0 ) {
                    result.error = qsTr("No function to call")
                }

                return result;
            }


            onClicked: {
                var result = check()
                if ( result.error !== null ) {
                    errorDialog.msg = result.error
                    errorDialog.open()
                    return
                }

                contractReady(encodedText.text, true)
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
