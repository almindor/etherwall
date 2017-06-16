/*
    This file is part of etherwall.
    etherwall is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    etherwall is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with etherwall. If not, see <http://www.gnu.org/licenses/>.
*/
/** @file PasswordDialog.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Password dialog
 */

import QtQuick 2.0
import QtQuick.Controls 1.1

BaseDialog {
    width: Math.max(parent.width * 0.6, 6 * dpi)
    property string password
    property bool acceptEmpty: true
    property bool wasAccepted: false
    signal rejected

    function openFocused(m, ae) {
        title = m || "Confirm operation"
        open()
        accountPW.focus = true
    }

    function doAccept() {
        if ( !acceptEmpty && password.length == 0 ) {
            return;
        }

        wasAccepted = true
        close()
        accepted()
        accountPW.text = ""
        password = ""
    }

    onVisibleChanged: {
        if ( !visible && !wasAccepted ) {
            rejected()
        } else {
            wasAccepted = false
        }
    }

    Row {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0.1 * dpi
        x: 0.1 * dpi

        Keys.onEscapePressed: {
            close()
            accountPW.text = ""
            password = ""
        }

        Keys.onEnterPressed: doAccept()
        Keys.onReturnPressed: doAccept()

        Label {
            text: qsTr("Password: ")
        }

        TextField {
            id: accountPW
            echoMode: TextInput.Password
            width: parent.parent.width * 0.6
            focus: true

            onTextChanged: {
                password = text
            }
        }

        Button {
            text: "OK"
            onClicked: {
                doAccept()
            }
        }
    }
}
