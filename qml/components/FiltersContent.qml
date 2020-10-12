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

        TableViewBase {
            id: filterView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - parent.spacing - addButton.height
            columns: [["Name", width - 5.5 * dpi], ["Contract", 4.5 * dpi], ["Active", 1 * dpi]]
            model: filterModel

            onItemDoubleClicked: function() {
                if ( currentRow >= 0 ) {
                    details.display(filterView.currentRow)
                }
            }

            Menu {
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

            MouseArea {
                anchors.fill: parent
                propagateComposedEvents: true
                acceptedButtons: Qt.RightButton

                onReleased: rowMenu.popup()
            }
        }
    }
}
