import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12

Loader {
    Column {
        anchors.fill: parent
        anchors.margins: 0.05 * dpi
        anchors.topMargin: 0.1 * dpi

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
                height: parent.height / 2.0 - 0.025 * dpi

                onClicked: details.display(-1)
            }

            Button {
                id: deployButton
                anchors.topMargin: 0.05 * dpi
                text: qsTr("Deploy New Contract")
                anchors.top: addButton.bottom
                width: parent.width / 2.0
                height: parent.height / 2.0 - 0.025 * dpi

                onClicked: deploy.display()
            }

            Button {
                id: invokeButton
                anchors.leftMargin: 0.05 * dpi
                anchors.left: addButton.right
                text: qsTr("Invoke ") + contractModel.getName(contractView.currentRow)
                visible: contractView.currentRow >= 0
                width: parent.width / 2.0 - 0.05 * dpi
                height: parent.height

                onClicked: calls.display(contractView.currentRow)
            }
        }

        TableViewBase {
            id: contractView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - parent.spacing - controlsRow.height
            model: contractModel
            columns: [["Name", width - 9 * dpi], ["Token (ERC20)", 4 * dpi], ["Address", 5 * dpi]]
            onItemDoubleClicked: function() {
                if ( currentRow >= 0 ) {
                    calls.display(contractView.currentRow)
                }
            }

            Menu {
                id: rowMenu
                enabled: contractView.currentRow >= 0

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
                        var url = "https://" + (ipc.testnet ? "rinkeby." : "") + "etherscan.io/address/" + contractModel.getAddress(contractView.currentRow)
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

            MouseArea {
                anchors.fill: parent
                propagateComposedEvents: true
                acceptedButtons: Qt.RightButton

                onReleased: rowMenu.popup()
            }
        }
    }
}
