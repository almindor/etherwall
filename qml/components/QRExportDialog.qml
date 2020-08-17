import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.3 as D

Dialog {
    id: exportWindow
    standardButtons: Dialog.Close | Dialog.Save
    width: 6 * dpi
    height: 6 * dpi
    focus: true
    anchors.centerIn: parent
    title: qsTr("QR code for: ") + address
    property string address

    function display( val, addr ) {
        address = addr
        code.value = val
        open()
    }

    onAccepted: saveDialog.open()

    QRCode {
        id: code
        anchors.fill: parent
    }

    D.FileDialog {
        id: saveDialog
        selectFolder: false
        selectExisting: false
        selectMultiple: false
        folder: shortcuts.documents
        nameFilters: [ "PNG (*.png)", "All files (*)" ]
        onAccepted: {
            var path = helpers.localURLToString(fileUrl)
            if ( !code.save(path) ) {
                appWindow.showBadge(qsTr("Error saving QR code to ") + path)
            } else {
                appWindow.showBadge(qsTr("Address saved as QR Code to ") + path)
            }
            exportWindow.close()
        }
    }
}
