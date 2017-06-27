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
/** @file TrezorImportDialog.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2017
 *
 * Trezor import dialog
 */

import QtQuick 2.0
import QtQuick.Controls 1.1

ConfirmDialog {
    title: qsTr("Import accounts from TREZOR")
    minimumHeight: 1.5 * dpi
    height: 1.5 * dpi

    function open(_msg) {
        msg = _msg
        opened();
        visible = true;
    }

    onYes: accountModel.trezorImport(offsetSpin.value, countSpin.value)

    Row {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 0.1 * dpi
        spacing: 0.3 * dpi

        Label {
            text: qsTr("Offset")
        }

        SpinBox {
            id: offsetSpin
            minimumValue: 0
            maximumValue: 4294967295 - countSpin.value
            value: 0
        }

        Label {
            text: qsTr("Count")
        }

        SpinBox {
            id: countSpin
            minimumValue: 1
            maximumValue: 255
            value: 5
        }
    }
}
