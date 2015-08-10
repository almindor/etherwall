import QtQuick 2.0
import QtQuick.Controls 1.0

Tab {
    id: accountsTab
    title: "Accounts"

    Column {
        id: col
        anchors.fill: parent

        Button {
            id: newAccountButton
            text: "New account"
        }

        TableView {
            id: accountView
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - newAccountButton.height - parent.spacing

            TableViewColumn {
                role: "hash"
                title: "Hash"
                width: 400
            }
            TableViewColumn {
                role: "balance"
                title: "Balance (Ether)"
                width: 150
            }
            model: accountModel
        }
    }
}
