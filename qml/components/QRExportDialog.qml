import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2

Dialog {
    id: exportWindow
    standardButtons: StandardButton.Close | StandardButton.Save
    width: 6 * dpi
    height: 6 * dpi
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

    FileDialog {
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
