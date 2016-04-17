import QtQuick 2.0
import QtQuick.Controls 1.1
import QtGraphicalEffects 1.0

Item {
    id: toolTipRoot
    width: toolTip.contentWidth
    height: toolTipContainer.height
    visible: false
    clip: false
    z: 999999999

    property alias text: toolTip.text
    property alias radius: content.radius
    property alias backgroundColor: content.color
    property alias textColor: toolTip.color
    property alias font: toolTip.font
    property var target: null

    function onMouseHover(x, y)
    {
        var obj = toolTipRoot.target.mapToItem(toolTipRoot.parent, x, y);
        toolTipRoot.x = obj.x;
        toolTipRoot.y = obj.y + 5;
    }

    function onVisibleStatus(flag)
    {
        toolTipRoot.visible = flag;
    }

    Component.onCompleted: {
        var itemParent = toolTipRoot.target;

        var newObject = Qt.createQmlObject('import QtQuick 2.0; MouseArea {signal mouserHover(int x, int y); signal showChanged(bool flag); anchors.fill:parent; hoverEnabled: true; onPositionChanged: {mouserHover(mouseX, mouseY)} onEntered: {showChanged(true)} onExited:{showChanged(false)} onClicked:{parent.focus = true}}',
                                           itemParent, "mouseItem");
        newObject.mouserHover.connect(onMouseHover);
        newObject.showChanged.connect(onVisibleStatus);
    }

    Item {
        id: toolTipContainer
        z: toolTipRoot.z + 1
        width: content.width + (2*toolTipShadow.radius)
        height: content.height + (2*toolTipShadow.radius)

        Rectangle {
            id: content
            anchors.centerIn: parent
            width: toolTipRoot.width
            height: toolTip.contentHeight + 10
            radius: 3

            Text {
                id: toolTip
                anchors {fill: parent; margins: 5}
                wrapMode: Text.WrapAnywhere
            }
        }
    }

    DropShadow {
        id: toolTipShadow
        z: toolTipRoot.z + 1
        anchors.fill: source
        cached: true
        horizontalOffset: 4
        verticalOffset: 4
        radius: 8.0
        samples: 16
        color: "#80000000"
        smooth: true
        source: toolTipContainer
    }

    Behavior on visible { NumberAnimation { duration: 200 }}
}
