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
/** @file SettingsTab.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Settings tab
 */

import QtQuick 2.0
import QtQuick.Controls 1.1

Tab {
    id: settingsTab
    title: qsTr("Settings")

    SettingsContent {
        id: settingsContent
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 0.05 * dpi
        anchors.topMargin: 0.1 * dpi
    }
}
