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

import QtQuick 2.12
import QtQuick.Controls 2.15

Dialog {
    // modality: Qt.WindowModal
    width: 5 * dpi
    standardButtons: Dialog.Apply | Dialog.Cancel
    anchors.centerIn: parent

    focus: true
    signal acceptedInput(string value)
    property string query: qsTr("Value: ", "Generic query question")

    function openFocused(m) {
        title = m || "Confirm operation"
        inputField.focus = true
        open()
    }

    onAccepted: {
        if ( inputField.text.length === 0 ) {
            return;
        }

        close()
        acceptedInput(inputField.text)
        inputField.text = ""
    }

    onApplied: accept()

    Row {
        anchors.centerIn: parent
        Keys.onEscapePressed: {
            close()
            inputField.text = ""
        }

        Keys.onEnterPressed: accept()
        Keys.onReturnPressed: accept()

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
