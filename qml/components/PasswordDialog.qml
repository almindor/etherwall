import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Dialogs 1.2

Dialog {
    title: qsTr("Confirm operation")
    width: Math.max(parent.width * 0.6, 500)
    property string password

    function openFocused() {
        open()
        accountPW.focus = true
    }

    Row {
        Keys.onEscapePressed: {
            close()
        }

        Label {
            text: qsTr("Password: ")
        }

        TextField {
            id: accountPW
            echoMode: TextInput.Password
            width: accountNewDialog.width * 0.7
            focus: true

            onTextChanged: {
                password = text
            }
        }
    }
}
