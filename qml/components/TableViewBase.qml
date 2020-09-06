import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12

Column {
    property var onItemDoubleClicked : function() {}
    property var columnSizes : []
    property var columnNames : []

    anchors.left: parent.left
    anchors.right: parent.right

    HorizontalHeaderView {
        syncView: tableView
        model: columnNames
    }

    TableView {
        id: tableView
        anchors.left: parent.left
        anchors.right: parent.right
        onWidthChanged: forceLayout()
        columnWidthProvider: function (column) {
            if ( columnNames.length === 0 ) {
                return 1 * dpi
            }

            if ( columnSizes.length === 0 ) {
                return width / columnNames.length
            }

            return columnSizes[column] || 0
        }

        property int currentRow: -1

        delegate: Rectangle {
            implicitWidth: cellText.width + 0.2 * dpi
            implicitHeight: 0.5 * dpi
            color: row === currentRow ? Universal.baseLowColor : Universal.altLowColor
            border {
                color: Universal.chromeBlackLowColor
                width: 1
            }

            MouseArea {
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
