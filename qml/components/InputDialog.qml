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
/** @file InputDialog.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * Text input dialog
 */

import QtQuick 2.0
import QtQuick.Controls 1.1

BaseDialog {
    width: Math.max(parent.width * 0.6, 500)
    property string value
    property string query: qsTr("Value: ", "Generic query question")

    function openFocused(m, ae) {
        title = m || "Confirm operation"
        open()
        inputField.focus = true
    }

    function doAccept() {
        if ( value.length == 0 ) {
            return;
        }

        close()
        accepted()
        inputField.text = ""
        value = ""
    }

    Row {
        anchors.centerIn: parent
        Keys.onEscapePressed: {
            close()
            inputField.text = ""
            value = ""
        }

        Keys.onEnterPressed: doAccept()
        Keys.onReturnPressed: doAccept()

        Label {
            text: qsTr(query)
        }

        TextField {
            id: inputField
            width: parent.parent.width * 0.6
            focus: true

            onTextChanged: {
                value = text
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
