import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Window 2.0
import QtQuick.Dialogs 1.2

Window {
    id: exportWindow
    width: 6 * dpi
    height: 7 * dpi
    title: qsTr("QR code for: ") + address
    property string address

    function open( val, addr ) {
        address = addr
        code.value = val
        visible = true
    }

    QRCode {
        id: code
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: saveButton.top
        anchors.top: parent.top
    }

    FileDialog {
        id: saveDialog
        selectFolder: false
        selectExisting: false
        selectMultiple: false
        folder: shortcuts.documents
        nameFilters: [ "PNG (*.png)", "All files (*)" ]
        onAccepted: {
            var path = helpers.localURLToString(fileUrl)
            code.save(path)
            appWindow.showBadge(qsTr("Address saved as QR Code to ") + path)
            exportWindow.close()
        }
    }

    Button {
        id: saveButton
        width: parent.width

        height: 1 * dpi
        anchors.bottom: parent.bottom
        text: "Save"
        onClicked: saveDialog.open()
    }
}
