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

import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

Dialog {
    id: contractDeploy
    title: qsTr("Deploy Contract")
    standardButtons: StandardButton.Cancel
    modality: Qt.WindowModal
    visible: false
    width: 7 * dpi
    height: 5.5 * dpi

    function display() {
        stcTab.active = true
        stcTab.children[0].toAddress = ""
        stcTab.children[0].contractData = "0x"
        stcTab.children[0].contractName = ""
        stcTab.children[0].contractAbi = ""
        stcTab.children[0].tokenIndex = 0
        stcTab.children[0].tokenAddress = ""
        stcTab.children[0].functionIsConstant = false
        stcTab.enabled = false
        stcTab.children[0].prepare()

        cccTab.active = true
        //cccTab.children[0].contractIndex = index
        tabs.currentIndex = 0
        open()
    }

    Badge {
        id: cdBadge
        z: 999

        Connections {
            target: trezor
            onButtonRequest: {
                if ( code === 8 && contractDeploy.visible ) {
                    cdBadge.show(cdBadge.button_msg(code))
                }
            }
        }
    }

    TabView {
        id: tabs
        anchors.fill: parent

        Tab {
            id: cccTab
            title: qsTr("Contract")
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

        Tab {
            id: stcTab
            title: qsTr("Transaction")
            SendTransactionContent {
                onDone: {
                    contractDeploy.close()
                }
            }
        }
    }
}
