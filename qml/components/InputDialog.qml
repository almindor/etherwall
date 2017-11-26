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
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.1

Dialog {
    modality: Qt.NonModal
    width: 5 * dpi
    standardButtons: StandardButton.Apply | StandardButton.Cancel
    signal acceptedInput(string value)
    property string query: qsTr("Value: ", "Generic query question")

    function openFocused(m) {
        title = m || "Confirm operation"
        open()
        inputField.focus = true
    }

    onApply: {
        if ( inputField.text.length === 0 ) {
            return;
        }

        close()
        acceptedInput(inputField.text)
        inputField.text = ""
    }

    Row {
        anchors.centerIn: parent
        Keys.onEscapePressed: {
            close()
            inputField.text = ""
        }

        Keys.onEnterPressed: apply()
        Keys.onReturnPressed: apply()

        Label {
            text: qsTr(query)
        }

        TextField {
            id: inputField
            width: parent.parent.width * 0.6
            focus: true
        }
    }
}
