import QtQuick 2.2
import QtQuick.Controls 1.2

Rectangle {
    id: badge

    property alias text: label.text
    property var stack : []

    visible: opacity !== 0.0
    opacity: 0.0
    border.color: "black"
    border.width: 1

    function show(value) {
        showTimer.stop()
        stack.push(value)
        if ( visible ) {
            return
        }

        text = stack.pop()
        opacity = 1.0
        hideTimer.start()
    }

    function hide() {
        opacity = 0.0
        hideTimer.stop()
    }

    Behavior on opacity {NumberAnimation{}}

    anchors.centerIn: parent

    SystemPalette { id: myPalette; colorGroup: SystemPalette.Active }

    color: myPalette.highlight

    height: 1 * dpi

    width: Math.max(label.paintedWidth + 1 * dpi, 4 * dpi)

    Timer {
        id: showTimer
        interval: 1000
        running: false
        repeat: false
        onTriggered: {
            show(stack.pop())
        }
    }

    Timer {
        id: hideTimer
        interval: 3000
        running: false
        repeat: false
        onTriggered: {
            if ( stack.length ) {
                showTimer.start()
            }
            hide()
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: hide()
    }

    Label {
        id: label
        color: "black"
        anchors.centerIn: parent
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
}
