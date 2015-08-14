import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Dialogs 1.2

Dialog {
    title: qsTr("Error")
    property string error

    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        text: error
        wrapMode: Text.Wrap
        width: parent.width

        Keys.onEscapePressed: {
            close()
        }
    }
}
