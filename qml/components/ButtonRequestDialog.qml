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
/** @file ButtonRequestDialog.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2017
 *
 * ButtonRequest dialog
 */

import QtQuick 2.0
import QtQuick.Controls 1.1

BaseDialog {
    title: qsTr("Button request from TREZOR")

    function open(code) {
        var code_map = {
            1:  qsTr("Other", "button request type"),
            2:  qsTr("Fee over treshold", "button request type"),
            3:  qsTr("Confirm output", "button request type"),
            4:  qsTr("Reset device", "button request type"),
            5:  qsTr("Confirm word", "button request type"),
            6:  qsTr("Wipe device", "button request type"),
            7:  qsTr("Protect Call", "button request type"),
            8:  qsTr("Sign Transaction", "button request type"),
            9:  qsTr("Firmware check", "button request type"),
            10: qsTr("Address", "button request type"),
            11: qsTr("Public Key", "button request type"),
        }

        msg = qsTr("Confirm operation on TREZOR: ") + code_map[code]
        show()
    }

    Button {
        text: "OK"
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 0.1 * dpi
        onClicked: {
           close()
           accepted()
        }
    }
}
