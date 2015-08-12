import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Dialogs 1.2

Dialog {
    title: qsTr("Error")
    width: Math.max(parent.width * 0.6, 500)
    property string error

    onErrorChanged: {
        open()
    }

    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        text: error

        Keys.onEscapePressed: {
            accountNewDialog.close()
        }
    }
}
