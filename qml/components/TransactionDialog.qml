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
/** @file TransactionDialog.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * FirstTime dialog
 */

import QtQuick 2.12
import QtQuick.Controls 2.15

Dialog {
    id: sendDialog
    title: qsTr("Send Ether")
    standardButtons: Dialog.Cancel
    // modality: Qt.WindowModal
    visible: false
    width: 8 * dpi
    height: 7 * dpi
    focus: true
    anchors.centerIn: parent

    function display() {
        stc.toAddress = ""
        stc.contractData = ""
        stc.contractName = ""
        stc.contractAbi = "[]"
        stc.tokenAddress = ""

        stc.prepare()
        open()
    }

    SendTransactionContent {
        id: stc
        contractData: ""
        contractAbi: ""
        contractName: ""
        onDone: sendDialog.close()
    }

    Badge {
        id: sdBadge
        z: 999

        Connections {
            target: trezor
            function onButtonRequest(code) {
                if ( code === 8 && sendDialog.visible ) {
                    sdBadge.show(sdBadge.button_msg(code))
                }
            }
        }
    }
}
