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

import QtQuick 2.12
import QtQuick.Controls 2.15

Dialog {
    // modality: Qt.platform.os === "osx" ? Qt.ApplicationModal : Qt.WindowModal // mac overlap bug
    id: theDialog
    width: 6 * dpi
    title: qsTr("Import accounts from TREZOR")
    standardButtons: Dialog.Yes | Dialog.No | Dialog.Help
    focus: true
    anchors.centerIn: parent

    onAccepted: accountModel.trezorImport(offsetSpin.value, countSpin.value)

    onHelpRequested: Qt.openUrlExternally("https://www.etherwall.com/faq/#importaccount")

    function display(_msg) {
        mainLabel.text = _msg
        open()
    }

    Column {
        anchors.fill: parent
        spacing: 0.2 * dpi

        Label {
            id: mainLabel
            width: parent.width
            wrapMode: Text.Wrap
        }

        Row {
            width: parent.width
            spacing: 0.3 * dpi

            Label {
                text: qsTr("Offset")
            }

            SpinBox {
                id: offsetSpin
                validator: IntValidator {
                    bottom: 0
                    top: 4294967295 - countSpin.value
                }

                value: 0
            }

            Label {
                text: qsTr("Count")
            }

            SpinBox {
                id: countSpin
                validator: IntValidator {
                    bottom: 1
                    top: 255
                }
                value: 5
            }
        }
    }
}
