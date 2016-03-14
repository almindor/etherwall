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
/** @file ConfirmDialog.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * Sync dialog
 */

import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Window 2.0

Window {
    title: Math.max(0, last - current) + " out of " + Math.max(0, last - first) + " blocks remaining to synchronize. "
    property int first
    property int last
    property int current

    modality: Qt.ApplicationModal
    visible: false
    minimumWidth: 6 * dpi
    minimumHeight: 1 * dpi
    maximumWidth: 6 * dpi
    maximumHeight: 1 * dpi
    width: 6 * dpi
    height: 1 * dpi
    x: Screen.width / 2.0 - width / 2.0
    y: Screen.height / 2.0 - height / 2.0
    flags: Qt.WindowCloseButtonHint

    ProgressBar {
        anchors.fill: parent
        value: Math.max(0, current - first) / Math.max(1, last - first)
    }
}
