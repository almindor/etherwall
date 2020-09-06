import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12

Loader {
    Column {
        anchors.fill: parent
        anchors.margins: 0.05 * dpi
        anchors.topMargin: 0.1 * dpi

        FilterDetails {
            id: details
        }

        Button {
            id: addButton
            text: qsTr("Add Watch")
            width: parent.width
            height: 1 * dpi

            onClicked: details.display()
        }

        HorizontalHeaderView {
            syncView: filterView
            model: ["Name", "Contract", "Active"]
        }

        TableView {
            id: filterView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - parent.spacing - addButton.height // *
            onWidthChanged: forceLayout()
            columnWidthProvider: function (column) { // *
                switch (column) {
                    case 0: return width - 5.5 * dpi
                    case 1: return 4.5 * dpi
                    case 2: return 1 * dpi
                }

                return 0
            }

            property int currentRow: -1

            delegate: Rectangle {
                implicitWidth: cellText.width + 0.2 * dpi
                implicitHeight: 0.5 * dpi
                color: row === filterView.currentRow ? Universal.baseLowColor : Universal.altLowColor
                border {
                    color: Universal.chromeBlackLowColor
                    width: 1
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: filterView.currentRow = row
                    onDoubleClicked: if ( filterView.currentRow >= 0 ) { // *
                        details.display(filterView.currentRow)
                    }
                }

                Text {
                    id: cellText
                    anchors.centerIn: parent
                    text: display
                }
            }

            model: filterModel // *

            Menu { // *
                id: rowMenu
                enabled: filterView.currentRow >= 0

                MenuItem {
                    text: qsTr("Activate/Deactivate")
                    onTriggered: {
                        filterModel.setFilterActive(filterView.currentRow, !filterModel.getActive(filterView.currentRow))
                    }
                }

                MenuItem {
                    text: qsTr("Edit")
                    onTriggered: {
                        details.display(filterView.currentRow)
                    }
                }

                MenuItem {
                    text: qsTr("Delete")
                    onTriggered: {
                        filterModel.deleteFilter(filterView.currentRow)
                    }
                }
            }

            MouseArea { // *
                anchors.fill: parent
                propagateComposedEvents: true
                acceptedButtons: Qt.RightButton

                onReleased: rowMenu.popup()
            }
        }
    }
}
