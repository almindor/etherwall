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
/** @file AccountDetails.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2017
 *
 * AccountDetails window
 */

import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Window 2.0
import QtQuick.Layouts 1.0

Window {
    modality: Qt.NonModal
    visible: false
    title: accountModel.selectedAccount
    width: 8 * dpi
    height: 4 * dpi
    minimumWidth: 8 * dpi
    minimumHeight: 4 * dpi

    Component.onCompleted: {
        setX(Screen.width / 2.0 - width / 2.0)
        setY(Screen.height / 2.0 - height / 2.0)
    }

    function open() {
        visible = true;
    }

    function close() {
        visible = false;
    }

    GridLayout {
        id: detailLayout
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 20
        columns: 2

        Label {
            text: qsTr("Address: ")
        }

        TextField {
            readOnly: true
            text: accountModel.selectedAccount
            Layout.minimumWidth: detailLayout.width * 0.75
        }

        Label {
            text: qsTr("Alias: ")
        }

        TextField {
            id: aliasField
            text: accountModel.selectedAccountAlias
            Layout.minimumWidth: detailLayout.width * 0.75
        }

        Label {
            text: qsTr("Sent Transactions: ")
        }

        TextField {
            readOnly: true
            text: accountModel.selectedAccountSentTrans
            Layout.minimumWidth: detailLayout.width * 0.75
        }

        Label {
            text: qsTr("DeviceID: ")
        }

        TextField {
            readOnly: true
            text: accountModel.selectedAccountDeviceID
            Layout.minimumWidth: detailLayout.width * 0.75
        }

        Label {
            text: qsTr("HD Path: ")
        }

        TextField {
            readOnly: true
            text: accountModel.selectedAccountHDPath
            Layout.minimumWidth: detailLayout.width * 0.75
        }

        CheckBox {
            id: defaultCheck
            enabled: false
            text: qsTr("Default")
            checked: accountModel.selectedAccountDefault
        }

        Button {
            text: qsTr("Set as default")
            onClicked: {
                accountModel.setAsDefault(accountModel.selectedAccount)
                defaultCheck.checked = true
            }
        }

        Label {
            text: qsTr("Operation: ")
        }

        Button {
            text: qsTr("Save")
            onClicked: {
                if ( aliasField.text.length ) {
                    accountModel.renameAccount(aliasField.text, accountView.selectedAccountRow);
                }
                close()
            }
        }
    }
}
