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
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.4

Dialog {
    property string password
    signal validPassword(string password)
    signal invalidPassword()
    standardButtons: StandardButton.Save | StandardButton.Cancel

    function checkMatch(pw1, pw2) {
        if ( accountPW0.text === accountPW1.text && accountPW0.text.length > 0 ) {
            pwcheck.color = "green"
            return true
        }

        pwcheck.color = "red"
        return false
    }

    onAccepted: {
        if (checkMatch(accountPW0.text, accountPW1.text)) {
            validPassword(accountPW0.text)
        } else {
            invalidPassword()
        }
    }

    onVisibleChanged: {
        if ( visible ) {
            accountPW0.text = ""
            accountPW1.text = ""
            accountPW0.focus = true
        }
    }

    Column {
        width: 5 * dpi
        Row {
            Keys.onEscapePressed: {
                close()
                accountPW0.text = ""
                password = ""
            }

            Label {
                text: qsTr("Password: ")
                width: 1.2 * dpi
            }

            TextField {
                id: accountPW0
                echoMode: TextInput.Password
                width: parent.parent.width * 0.6
                onTextChanged: checkMatch(text, accountPW1.text)
            }

        }

        Row {
            Keys.onEscapePressed: {
                close()
                accountPW1.text = ""
                password = ""
            }

            Label {
                text: qsTr("Repeat: ", "password")
                width: 1.2 * dpi
            }

            Column {
                width: parent.parent.width * 0.6
                spacing: 1

                TextField {
                    id: accountPW1
                    width: parent.width
                    echoMode: TextInput.Password
                    onTextChanged: checkMatch(text, accountPW0.text)
                }

                Rectangle {
                    id: pwcheck
                    width: parent.width
                    height: 1
                    color: "white"
                }
            }
        }
    }
}
