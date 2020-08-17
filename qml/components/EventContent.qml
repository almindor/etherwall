import QtQuick 2.12
import QtQuick.Controls 1.4 as C
import QtQuick.Controls 2.12

Item {
    Column {
        anchors.fill: parent
        anchors.margins: 0.05 * dpi
        anchors.topMargin: 0.1 * dpi
        spacing: 0.1 * dpi

        EventDetails {
            id: details
        }

        C.TableView {
            id: eventView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - parent.spacing

            C.TableViewColumn {
                role: "name"
                title: qsTr("Event")
                width: 0.4 * parent.width
            }
            C.TableViewColumn {
                role: "contract"
                title: qsTr("Contract")
                width: 0.4 * parent.width
            }
            C.TableViewColumn {
                role: "blocknumber"
                title: qsTr("Block Number")
                width: 0.18 * parent.width
            }
            model: eventModel

            Menu {
                id: rowMenu

                MenuItem {
                    text: qsTr("Details")
                    onTriggered: {
                        details.display(eventView.currentRow)
                    }
                }

                MenuItem {
                    text: qsTr("Find on blockchain explorer")
                    onTriggered: {
                        var url = "https://" + (ipc.testnet ? "rinkeby." : "") + "etherscan.io/tx/" + eventModel.getTransactionHash(eventView.currentRow)
                        Qt.openUrlExternally(url)
                    }
                }
            }

            onDoubleClicked: {
                if ( eventView.currentRow >= 0 ) {
                    details.display(eventView.currentRow)
                }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                propagateComposedEvents: true
                onReleased: {
                    if ( parent.currentRow >= 0 ) {
                        rowMenu.popup();
                    }
                }
            }
        }
    }
}
