import QtQuick 2.0
import QtQuick.Controls 1.0
import "components"

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: qsTr("Etherwall Ethereum Wallet")

    TabView {
        anchors.fill: parent

        AccountsTab {}

        Tab {
            id: transactionsTab
            title: "Transactions"

            Row {
                anchors.left: parent.left
                anchors.right: parent.right

                TextField {
                    id: sendEdit
                    width: parent.width - sendButton.width
                    text: "send addr"
                }

                Button {
                    id: sendButton
                    text: "send"
                }
            }
        }

        Tab {
            id: settingsTab
            title: "Settings"
        }
    }
}
