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
/** @file ContractDeploy.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * FirstTime dialog
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Dialog {
    id: contractDeploy
    title: qsTr("Deploy Contract")
    standardButtons: Dialog.Cancel
    // modality: Qt.WindowModal
    visible: false
    width: 7 * dpi
    height: 7 * dpi
    focus: true
    anchors.centerIn: parent

    function display() {
        stcTab.children[0].toAddress = ""
        stcTab.children[0].contractData = "0x"
        stcTab.children[0].contractName = ""
        stcTab.children[0].contractAbi = ""
        stcTab.children[0].tokenAddress = ""
        stcTab.children[0].functionIsConstant = false
        stcTab.children[0].prepare()

        tabs.currentIndex = 0
        open()
    }

    Badge {
        id: cdBadge
        z: 999

        Connections {
            target: trezor
            function onButtonRequest(code) {
                if ( code === 8 && contractDeploy.visible ) {
                    cdBadge.show(cdBadge.button_msg(code))
                }
            }
        }
    }

    TabBar {
        id: tabs
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        TabButton {
            text: qsTr("Contract")
        }

        TabButton {
            text: qsTr("Transaction")
        }
    }

    StackLayout {
        anchors.top: tabs.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: tabs.currentIndex

        Item {
            id: cccTab
            DeployContractContent {
                onDone: {
                    contractDeploy.close()
                }

                onContractError: {
                    stcTab.enabled = false
                    tabs.currentIndex = 0
                }

                onContractReady: {
                    stcTab.children[0].contractData = encoded
                    stcTab.children[0].contractName = name
                    stcTab.children[0].contractAbi = abi
                    stcTab.enabled = true
                    stcTab.children[0].prepare()
                    if ( next ) {
                        tabs.currentIndex = 1
                    }
                }
            }
        }

        Item {
            id: stcTab
            SendTransactionContent {
                onDone: {
                    contractDeploy.close()
                }
            }
        }
    }
}
