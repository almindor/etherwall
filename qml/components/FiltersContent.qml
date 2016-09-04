import QtQuick 2.0
import QtQuick.Controls 1.2

Item {
    anchors.fill: parent

    Column {
        anchors.fill: parent
        anchors.margins: 0.05 * dpi
        anchors.topMargin: 0.1 * dpi
        spacing: 0.1 * dpi

        FilterDetails {
            id: details
        }

        Button {
            id: addButton
            text: qsTr("Add Watch")
            width: parent.width
            height: 1 * dpi

            onClicked: details.open()
        }

        TableView {
            id: filterView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - parent.spacing - addButton.height

            TableViewColumn {
                role: "name"
                title: qsTr("Name")
                width: 2.25 * dpi
            }
            TableViewColumn {
                role: "contract"
                title: qsTr("Contract")
                width: 4 * dpi
            }
            TableViewColumn {
                role: "active"
                title: qsTr("Active")
                width: 1 * dpi
            }
            model: filterModel

            Menu {
                id: rowMenu

                MenuItem {
                    text: qsTr("Activate/Deactivate")
                    onTriggered: {
                        filterModel.setFilterActive(filterView.currentRow, !filterModel.getActive(filterView.currentRow))
                    }
                }

                MenuItem {
                    text: qsTr("Edit")
                    onTriggered: {
                        details.open(filterView.currentRow)
                    }
                }

                MenuItem {
                    text: qsTr("Delete")
                    onTriggered: {
                        filterModel.deleteFilter(filterView.currentRow)
                    }
                }
            }

            onDoubleClicked: {
                if ( filterView.currentRow >= 0 ) {
                    details.open(filterView.currentRow)
                }
            }

            MouseArea {
                anchors.fill: parent
                propagateComposedEvents: true
                acceptedButtons: Qt.RightButton

                onReleased: {
                    if ( filterView.currentRow >= 0 ) {
                        rowMenu.popup()
                    }
                }
            }
        }
    }
}
