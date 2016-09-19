import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

Item {
    anchors.fill: parent

    signal done
    signal contractReady(string encoded, bool next)
    signal contractError

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
                text: qsTr("Name: ")
            }

            TextField {
                id: nameField
                width: mainColumn.width - 1 * dpi

                maximumLength: 255

                onTextChanged: deployButton.refresh()
            }
        }

        Row {
            Label {
                text: qsTr("ABI: ")
                width: 1 * dpi
            }

            TextArea {
                id: abiField
                width: mainColumn.width - 1 * dpi
                height: 1.0 * dpi

                onTextChanged: deployButton.refresh()
            }
        }

        Row {
            Label {
                text: qsTr("Bytecode: ")
                width: 1 * dpi
            }

            TextArea {
                id: bcField
                width: mainColumn.width - 1 * dpi
                height: 1.5 * dpi

                onTextChanged: deployButton.refresh()
            }
        }

        Button {
            id: deployButton
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

                return result
            }

            function refresh() {
                var result = check()
                if ( result.error !== null ) {

                }
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
