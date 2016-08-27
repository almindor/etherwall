import QtQuick 2.0
import QtQuick.Controls 1.2

Item {
    anchors.fill: parent

    Column {
        anchors.fill: parent
        anchors.margins: 0.05 * dpi
        anchors.topMargin: 0.1 * dpi
        spacing: 0.1 * dpi

        /*FilterDetails {
            id: details
        }*/

        TableView {
            id: filterView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - parent.spacing

            TableViewColumn {
                role: "name"
                title: qsTr("Event")
                width: 2.25 * dpi
            }
            TableViewColumn {
                role: "contract"
                title: qsTr("Contract")
                width: 2.25 * dpi
            }
            TableViewColumn {
                role: "blocknumber"
                title: qsTr("Block Number")
                width: 2.25 * dpi
            }
            model: eventModel
        }
    }
}
