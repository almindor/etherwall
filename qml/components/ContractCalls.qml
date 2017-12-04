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
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

Dialog {
    id: contractCalls
    title: qsTr("Call Contract")
    standardButtons: StandardButton.Close
    modality: Qt.WindowModal
    visible: false
    width: 7 * dpi
    height: 5.5 * dpi

    function display( index ) {
        rsTab.active = true

        stcTab.active = true
        stcTab.children[0].toAddress = contractModel.getAddress(index)
        stcTab.children[0].contractData = "0x"
        stcTab.children[0].contractName = ""
        stcTab.children[0].contractAbi = ""
        stcTab.children[0].tokenIndex = 0
        stcTab.children[0].tokenAddress = ""        

        cccTab.active = true
        cccTab.children[0].open(index) // ensure first function is selected ok
        stcTab.children[0].prepare()

        tabs.currentIndex = 0
        open()
    }

    Badge {
        id: ccBadge
        z: 999

        Connections {
            target: trezor
            onButtonRequest: {
                if ( code === 8 && contractCalls.visible ) {
                    ccBadge.show(ccBadge.button_msg(code))
                }
            }
        }

        Connections {
            target: ipc

            onCallDone: {
                if ( userData["type"] === "functionCall" ) {
                    tabs.currentIndex = 2
                }
            }
        }
    }

    TabView {
        id: tabs
        anchors.fill: parent

        Tab {
            id: cccTab
            title: qsTr("Function")
            CallContractContent {
                onDone: {
                    contractCalls.close()
                }

                onContractError: {
                    stcTab.enabled = false
                    tabs.currentIndex = 0
                }

                onContractReady: {
                    rsTab.children[0].callIndex = callIndex
                    rsTab.enabled = constant

                    stcTab.children[0].contractData = encoded
                    stcTab.children[0].functionIsConstant = constant
                    stcTab.children[0].callIndex = callIndex
                    stcTab.children[0].userData = userData
                    stcTab.enabled = true
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
                onDone: contractCalls.close()
            }
        }

        Tab {
            id: rsTab
            title: qsTr("Results")
            FunctionResultsContent {
                onDone: contractCalls.close()
            }
        }
    }
}
