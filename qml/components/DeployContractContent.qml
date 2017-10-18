import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

Item {
    anchors.fill: parent

    signal done
    signal contractReady(string name, string abi, string encoded, bool next)
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
                text: qsTr("Interface: ")
                width: 1 * dpi
            }

            TextArea {
                id: abiField
                width: mainColumn.width - 1 * dpi
                wrapMode: TextEdit.WrapAnywhere
                height: 1.0 * dpi

                onTextChanged: deployButton.refresh()
            }
        }

        Row {
            Label {
                id: byteCodeLabel
                text: qsTr("Bytecode:") + '<a href="http://www.etherwall.com/faq/#bytecode">?</a>'
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
                onLinkActivated: Qt.openUrlExternally(link)
                width: 1 * dpi
            }

            TextArea {
                id: bcField
                wrapMode: TextEdit.WrapAnywhere
                width: mainColumn.width - 1 * dpi
                height: 1.0 * dpi

                onTextChanged: deployButton.refresh()
            }
        }

        Row {
            id: errorRow
            visible: errorText.length > 0

            Label {
                text: qsTr("Status: ")
                width: 1 * dpi
            }

            TextField {
                id: errorText
                width: mainColumn.width - 1 * dpi
                readOnly: true
                property bool ready: false

                style: TextFieldStyle {
                    textColor: "black"
                    background: Rectangle {
                        radius: 2
                        border.color: errorText.ready ? "green" : "red"
                        border.width: 1
                    }
                }
            }

        }

        Button {
            id: deployButton
            width: parent.width
            height: 0.6 * dpi
            text: errorText.ready ? qsTr("Setup Transaction") : qsTr("Invalid Input")

            Image {
                id: callIcon
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: parent.height * 0.15
                width: height
                source: errorText.ready ? "/images/ok" : "/images/warning"
            }

            style: ButtonStyle {
              label: Text {
                renderType: Text.NativeRendering
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: deployButton.height / 2.0
                text: control.text
              }
            }

            function check() {
                var result = {
                    error: null
                }

                if ( !nameField.text.length ) {
                    result.error = "Name not defined"
                    return result
                }

                if ( !abiField.text.length ) {
                    result.error = "Interface not defined"
                    return result
                }

                if ( !bcField.text.length ) {
                    result.error = "Bytecode not defined"
                    return result
                }

                result.bc = bcField.text.trim()
                if ( !result.bc.match(/^(0\x)?[a-f,A-Z,0-9]+$/) || (result.bc.length % 2 !== 0) ) {
                    result.error = qsTr("Invalid bytecode")
                    return result
                }

                try {
                    var parsed = JSON.parse(abiField.text)
                    if ( !parsed || !parsed.length ) {
                        result.error = "API not an array"
                        return result
                    }

                    result.abi = abiField.text
                } catch ( err ) {
                    result.error = "Interface parse error: " + err
                    return result
                }

                return result
            }

            function refresh() {
                var result = check()
                if ( result.error !== null ) {
                    errorText.ready = false
                    errorText.text = result.error
                } else {
                    errorText.ready = true
                    errorText.text = qsTr("Ready")
                }
            }

            onClicked: {
                var result = check()
                if ( result.error !== null ) {
                    errorDialog.msg = result.error
                    errorDialog.open()
                    return
                }

                contractReady(nameField.text, result.abi, result.bc, true)
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
