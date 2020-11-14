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
/** @file ContractCalls.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * FirstTime dialog
 */

import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

Dialog {
    id: contractCalls
    title: qsTr("Call Contract")
    standardButtons: Dialog.Close
    // modality: Qt.WindowModal
    visible: false
    width: 7 * dpi
    height: 7 * dpi
    focus: true
    anchors.centerIn: parent

    function display( index ) {
        stcTab.children[0].toAddress = contractModel.getAddress(index)
        stcTab.children[0].contractData = "0x"
        stcTab.children[0].contractName = ""
        stcTab.children[0].contractAbi = ""
        stcTab.children[0].tokenAddress = ""

        cccTab.children[0].open(index) // ensure first function is selected ok

        open()
    }

    Badge {
        id: ccBadge
        z: 999

        Connections {
            target: trezor
            function onButtonRequest(code) {
                if ( code === 8 && contractCalls.visible ) {
                    ccBadge.show(ccBadge.button_msg(code))
                }
            }
        }

        Connections {
            target: ipc

            function onCallDone(result, index, userData) {
                if ( userData["type"] === "functionCall" ) {
                    tabs.currentIndex = 2
                }
            }
        }
    }

    TabBar {
        id: tabs
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right

        TabButton {
            text: qsTr("Function")
        }
        TabButton {
            text: qsTr("Transaction")
            enabled: false
        }
        TabButton {
            text: qsTr("Results")
        }
    }

    StackLayout {
        id: cccStack
        anchors.top: tabs.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: tabs.currentIndex

        Item {
            id: cccTab
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
                    stcTab.children[0].prepare()
                    stcTab.enabled = true
                    if ( next ) {
                        tabs.currentIndex = 1
                    }
                }
            }
        }

        Item {
            id: stcTab
            enabled: false
            SendTransactionContent {
                onDone: contractCalls.close()
            }
        }

        Item {
            id: rsTab
            FunctionResultsContent {
                onDone: contractCalls.close()
            }
        }
    }
}
