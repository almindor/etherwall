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
/** @file AccountsTab.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * Currency tab
 */

import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.0

Tab {
    id: logTab
    title: qsTr("Currencies")
    enabled: !ipc.busy && !ipc.starting && (ipc.connectionState > 0)

    Column {
        anchors.margins: 0.2 * dpi
        anchors.top: parent.top
        spacing: 0.1 * dpi

        Item {
            anchors.left: parent.left
            anchors.margins: 0.1 * dpi
            width: ccText.width + ccImg.width
            height: Math.max(ccImg.height, ccText.height)

            Text {
                id: ccText
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 0.2 * dpi
                textFormat: Text.RichText
                text: qsTr('Prices courtesy of ')
            }

            Image {
                id: ccImg
                anchors.left: ccText.right
                sourceSize.height: 0.5 * dpi
                source: "/images/cc"
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.OpenHandCursor
                onClicked: {
                    Qt.openUrlExternally("http://cryptocompare.com")
                }
            }
        }

        Row {
            spacing: 0.5 * dpi
            anchors.margins: 0.2 * dpi

            CurrencyRow {
                anchors.verticalCenter: parent.verticalCenter
                leftText: "1"
                rightText: ""
                currency: "ETH"
                heightInches: 1.7
            }

            Column {
                spacing: 0.3 * dpi
                anchors.verticalCenter: parent.verticalCenter

                Repeater {
                    model: currencyModel

                    delegate: CurrencyRow {
                        visible: index > 0
                        heightInches: 2.5 / currencyModel.count
                        leftText: "="
                        currency: currencyModel.getCurrencyName(index)
                        rightText: Number(currencyModel.getCurrencyPrice(index)).toFixed(5)
                    }
                }
            }
        }

    }

}
