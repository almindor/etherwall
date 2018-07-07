import QtQuick 2.0

Column {
    spacing: 5
    width: parent.width
    property alias label: textLabel.text

    Text {
        id: textLabel
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        font.bold: true
    }

    Rectangle {
        border.width: 1
        height: 2
        width: parent.width
        anchors.margins: 20
        border.color: "#2d2b19"
    }
}
