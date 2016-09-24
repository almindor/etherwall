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
/** @file ConfirmDialog.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Confirm dialog
 */

import QtQuick 2.0
import QtQuick.Controls 1.1

BaseDialog {
    title: qsTr("Confirm")
    property string yesText : qsTr("Yes")
    property string noText : qsTr("No")

    Row {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 0.1 * dpi

        Button {
            text: yesText
            onClicked: {
               close()
               yes()
            }
        }

        Button {
            text: noText
            onClicked: {
               close()
            }
        }
    }
}
