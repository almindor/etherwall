import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12

Item {
    property var columns: []
    property var model : nil
    property int currentRow: -1
    property real itemImplicitHeight: 0.5 * dpi

    property var onItemDoubleClicked: function() {}

    onWidthChanged: tableView.forceLayout()

    function refresh() {
        tableView.forceLayout()
    }

    HorizontalHeaderView {
        id: headerView
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        syncView: tableView
        model: parent.columns.map(c => c[0])
    }

    TableView {
        id: tableView
        anchors.top: headerView.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        implicitHeight: parent.height
        onWidthChanged: forceLayout()
        columnWidthProvider: function (column) {
            if ( parent.columns.length === 0 ) {
                return 1 * dpi
            }

            return parent.columns[column][1]
        }

        model: parent.model

        delegate: Rectangle {
            implicitWidth: cellText.width + 0.2 * dpi
            implicitHeight: itemImplicitHeight
            color: row === currentRow ? Universal.baseLowColor : Universal.altLowColor
            border {
                color: Universal.chromeBlackLowColor
                width: 1
            }

            MouseArea {
                id: itemArea
                anchors.fill: parent
                onClicked: currentRow = row
                onDoubleClicked: onItemDoubleClicked()
            }

            Text {
                id: cellText
                anchors.centerIn: parent
                text: display
            }
        }
    }

}
