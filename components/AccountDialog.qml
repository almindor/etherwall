import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Dialogs 1.2

Dialog {
    title: qsTr("Confirm account")
    width: Math.max(parent.width * 0.6, 500)
    property string password

    Row {
        Keys.onEscapePressed: {
            accountNewDialog.close()
        }

        Label {
            text: qsTr("Password: ")
        }

        TextField {
            id: accountPW
            echoMode: TextInput.Password
            width: accountNewDialog.width * 0.7
            onTextChanged: {
                password = text
            }
        }
    }
}
