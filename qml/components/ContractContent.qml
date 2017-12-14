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

                onClicked: details.display(-1)
            }

            Button {
                id: deployButton
                text: qsTr("Deploy New Contract")
                anchors.top: addButton.bottom
                width: parent.width / 2.0
                height: parent.height / 2.0

                onClicked: deploy.display()
            }

            Button {
                id: invokeButton
                anchors.left: addButton.right
                text: qsTr("Invoke ") + contractModel.getName(contractView.currentRow)
                visible: contractView.currentRow >= 0
                width: parent.width / 2.0
                height: parent.height

                onClicked: calls.display(contractView.currentRow)
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
                width: 0.33 * parent.width
            }
            TableViewColumn {
                role: "token"
                title: qsTr("Token (ERC20)")
                width: 0.2 * parent.width
            }
            TableViewColumn {
                role: "address"
                title: qsTr("Address")
                width: 0.45 * parent.width
            }

            model: contractModel

            Menu {
                id: rowMenu

                MenuItem {
                    text: qsTr("Invoke")
                    onTriggered: calls.display(contractView.currentRow)
                }

                MenuItem {
                    text: qsTr("Edit")
                    onTriggered: details.display(contractView.currentRow)
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
                    onTriggered: clipboard.setText(contractModel.getAddress(contractView.currentRow))
                }

                MenuItem {
                    text: qsTr("Delete")
                    onTriggered: contractModel.deleteContract(contractView.currentRow)
                }
            }

            onDoubleClicked: {
                if ( contractView.currentRow >= 0 ) {
                    calls.display(contractView.currentRow)
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
