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

    function button_msg(code) {
        var code_map = {
            1:  qsTr("Other", "button request type"),
            2:  qsTr("Fee over treshold", "button request type"),
            3:  qsTr("Confirm output", "button request type"),
            4:  qsTr("Reset device", "button request type"),
            5:  qsTr("Confirm word", "button request type"),
            6:  qsTr("Wipe device", "button request type"),
            7:  qsTr("Protect Call", "button request type"),
            8:  qsTr("Sign Transaction", "button request type"),
            9:  qsTr("Firmware check", "button request type"),
            10: qsTr("Address", "button request type"),
            11: qsTr("Public Key", "button request type"),
        }

        return qsTr("Confirm operation on TREZOR: ") + code_map[code]
    }

    Behavior on opacity {NumberAnimation{}}

    anchors.centerIn: parent

    SystemPalette { id: myPalette; colorGroup: SystemPalette.Active }

    color: myPalette.highlight

    height: 1 * dpi
    width: parent.width * 0.95

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

    Text {
        id: label
        color: "black"
        wrapMode: Text.Wrap
        width: parent.width * 0.9
        anchors.centerIn: parent
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
}
