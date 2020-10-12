import QtQuick 2.12
import QtQuick.Controls 2.15

Loader {
    anchors.fill: parent

    Column {
        anchors.fill: parent
        anchors.margins: 0.05 * dpi
        anchors.topMargin: 0.1 * dpi

        EventDetails {
            id: details
        }

        TableViewBase {
            id: eventView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - parent.spacing
            itemImplicitHeight: 0.5 * dpi
            model: eventModel
            columns: [["Name", 3 * dpi], ["Contract", width - 4 * dpi], ["Block#", 1 * dpi]]
            onItemDoubleClicked: function() {
                if ( currentRow >= 0 ) {
                    details.display(currentRow)
                }
            }

            Menu {
                id: rowMenu
                enabled: parent.currentRow >= 0

                MenuItem {
                    text: qsTr("Details")
                    onTriggered: {
                        details.display(eventView.currentRow)
                    }
                }

                MenuItem {
                    text: qsTr("Find on blockchain explorer")
                    onTriggered: {
                        var url = "https://" + (ipc.testnet ? "rinkeby." : "") + "etherscan.io/tx/" + eventModel.getTransactionHash(eventView.currentRow) + "#eventlog"
                        Qt.openUrlExternally(url)
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                propagateComposedEvents: true
                onReleased: rowMenu.popup()
            }
        }
    }
}
