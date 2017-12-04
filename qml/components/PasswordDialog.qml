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
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.1

Dialog {
    id: theDialog
    title: qsTr("Confirm operation", "generic dialog")
    width: dpi * 7
    standardButtons: StandardButton.Yes | StandardButton.No
    modality: Qt.WindowModal
    property string text : ""
    signal passwordSubmitted(string password)
    signal passwordRejected

    function openFocused(m) {
        title = m || title
        open()
        accountPW.focus = true
    }

    onYes: {
        passwordSubmitted(accountPW.text)
        accountPW.text = ""
    }

    onNo: {
        passwordRejected()
        accountPW.text = ""
    }

    function doYes() {
        yes()
        close()
    }

    function doNo() {
        no()
        close()
    }

    Column {
        width: parent.width

        Keys.onEnterPressed: doYes()
        Keys.onReturnPressed: doYes()
        Keys.onEscapePressed: doNo()

        Label {
            text: theDialog.text
            visible: theDialog.text.length > 0
        }

        Row {
            Label {
                text: qsTr("Password: ")
            }

            TextField {
                id: accountPW
                echoMode: TextInput.Password
                width: parent.parent.width * 0.6
                focus: true
            }
        }
    }

}
