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
 * Info tab
 */

import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

Loader {
    id: infoTab

    Item {
        anchors.fill: parent

        TabBar {
            id: infoTabBar
            anchors.left: parent.left
            anchors.right: parent.right
            position: TabBar.Footer

            TabButton {
                text: qsTr("Application")
            }

            TabButton {
                text: qsTr("Geth")
            }
        }

        StackLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: infoTabBar.bottom
            anchors.bottom: parent.bottom

            currentIndex: infoTabBar.currentIndex

            LogTab {}
            GethTab {}
        }
    }
}
