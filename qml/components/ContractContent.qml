import QtQuick 2.0
import QtQuick.Controls 1.2

Item {
    anchors.fill: parent

    Column {
        anchors.fill: parent
        anchors.margins: 0.05 * dpi
        anchors.topMargin: 0.1 * dpi
        spacing: 0.1 * dpi

        ContractDetails {
            id: details
        }

        ContractCalls {
            id: calls
        }

        ContractDeploy {
            id: deploy
        }

        Item {
            id: controlsRow
            height: 1 * dpi
            width: parent.width

            Button {
                id: addButton
                text: qsTr("Add Existing Contract")
                width: parent.width / 2.0
                height: parent.height / 2.0

                onClicked: details.open(-1)
            }

            Button {
                id: deployButton
                text: qsTr("Deploy New Contract")
                anchors.top: addButton.bottom
                width: parent.width / 2.0
                height: parent.height / 2.0

                onClicked: deploy.open()
            }

            Button {
                id: invokeButton
                anchors.left: addButton.right
                text: qsTr("Invoke ") + contractModel.getName(contractView.currentRow)
                visible: contractView.currentRow >= 0
                width: parent.width / 2.0
                height: parent.height

                onClicked: calls.open(contractView.currentRow)
            }
        }

        TableView {
            id: contractView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - parent.spacing - controlsRow.height

            TableViewColumn {
                role: "name"
                title: qsTr("Name")
                width: 2.25 * dpi
            }
            TableViewColumn {
                role: "address"
                title: qsTr("Address")
                width: 4 * dpi
            }

            model: contractModel

            Menu {
                id: rowMenu

                MenuItem {
                    text: qsTr("Invoke")
                    onTriggered: {
                        calls.open(contractView.currentRow)
                    }
                }

                MenuItem {
                    text: qsTr("Edit")
                    onTriggered: {
                        details.open(contractView.currentRow)
                    }
                }

                MenuItem {
                    text: qsTr("Find on blockchain explorer")
                    onTriggered: {
                        var url = "http://" + (ipc.testnet ? "rinkeby." : "") + "etherscan.io/address/" + contractModel.getAddress(contractView.currentRow)
                        Qt.openUrlExternally(url)
                    }
                }

                MenuItem {
                    text: qsTr("Copy Address")
                    onTriggered: {
                        clipboard.setText(contractModel.getAddress(contractView.currentRow))
                    }
                }

                MenuItem {
                    text: qsTr("Delete")
                    onTriggered: {
                        contractModel.deleteContract(contractView.currentRow)
                    }
                }
            }

            onDoubleClicked: {
                if ( contractView.currentRow >= 0 ) {
                    calls.open(contractView.currentRow)
                }
            }

            MouseArea {
                anchors.fill: parent
                propagateComposedEvents: true
                acceptedButtons: Qt.RightButton

                onReleased: {
                    if ( contractView.currentRow >= 0 ) {
                        rowMenu.popup()
                    }
                }
            }
        }
    }
}
