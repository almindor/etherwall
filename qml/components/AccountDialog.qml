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
/** @file AccountDialog.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * Account dialog
 */

import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.4

BaseDialog {
    width: Math.max(parent.width * 0.6, 6 * dpi)
    property string password

    function openFocused(m, ae) {
        accountPW0.text = ""
        accountPW1.text = ""
        password = ""

        title = m || "Confirm operation"
        open()
        accountPW0.focus = true
    }

    function doAccept() {
        if ( password.length == 0 ) {
            return;
        }

        close()
        accepted()
        accountPW0.text = ""
        accountPW1.text = ""
        password = ""
    }

    function submit() {
        if (accountPW0.text === accountPW1.text && accountPW0.text.length > 0) {
            password = accountPW0.text
            doAccept()
        }
    }

    Row {
        id: topRow
        anchors.bottom: bottomRow.top
        anchors.bottomMargin: 0.1 * dpi
        x: 0.1 * dpi

        Keys.onEscapePressed: {
            close()
            accountPW0.text = ""
            password = ""
        }

        Keys.onEnterPressed: doAccept()
        Keys.onReturnPressed: doAccept()

        Label {
            text: qsTr("Password: ")
        }

        TextField {
            id: accountPW0
            echoMode: TextInput.Password
            width: parent.parent.width * 0.6
            focus: true
        }
    }

    Row {
        id: bottomRow
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0.1 * dpi
        x: 0.1 * dpi

        Keys.onEscapePressed: {
            close()
            accountPW1.text = ""
            password = ""
        }

        Keys.onEnterPressed: doAccept()
        Keys.onReturnPressed: doAccept()

        Label {
            text: qsTr("Repeat: ", "password") + "    "
        }

        function submit() {

        }

        TextField {
            id: accountPW1
            echoMode: TextInput.Password
            width: parent.parent.width * 0.6
            Keys.onReturnPressed: submit()
            Keys.onEnterPressed: submit()
        }

        Button {
            id: okButton
            text: "OK"
            enabled: accountPW0.text === accountPW1.text && accountPW0.text.length > 0
            onClicked: submit()
        }
    }
}
