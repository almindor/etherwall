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
/** @file FirstTimeDialog.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * FirstTime dialog
 */

import QtQuick 2.0
import QtQuick.Window 2.0

Window {
    id: sendDialog
    title: qsTr("Send Ether")

    modality: Qt.NonModal
    visible: false
    minimumWidth: 6 * dpi
    minimumHeight: 1 * dpi
    maximumWidth: 10 * dpi
    maximumHeight: 8 * dpi
    width: 7 * dpi
    height: 5 * dpi
    Component.onCompleted: {
        setX(Screen.width / 2.0 - width / 2.0)
        setY(Screen.height / 2.0 - height / 2.0)
    }

    SendTransactionContent {
        contractData: ""
        contractAbi: ""
        contractName: ""
        onDone: {
            sendDialog.close()
        }
    }

    Badge {
        id: sdBadge
        z: 999

        Connections {
            target: trezor
            onButtonRequest: {
                if ( code === 8 && sendDialog.visible ) {
                    sdBadge.show(sdBadge.button_msg(code))
                }
            }
        }
    }
}
