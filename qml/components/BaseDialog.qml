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
/** @file BaseDialog.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Base dialog
 */

import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Window 2.0

Window {
    id: baseDialogID
    signal accepted
    signal yes
    signal opened

    property string msg

    modality: Qt.ApplicationModal
    visible: false
    width: 6 * dpi
    height: 1 * dpi
    minimumWidth: 6 * dpi
    minimumHeight: 1 * dpi

    function open() {
        baseDialogID.opened();
        visible = true;
    }

    function close() {
        visible = false;
    }

    onVisibleChanged: {
        setX(Screen.width / 2.0 - width / 2.0)
        setY(Screen.height / 2.0 - height / 2.0)
    }

    Label {
        id: contentLabel
        y: 0.1 * dpi
        x: 0.1 * dpi
        text: msg
        wrapMode: Text.Wrap
        width: parent.width
        onLinkActivated: Qt.openUrlExternally(link)

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
            cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
        }

        Keys.onEscapePressed: {
            close()
        }
    }
}
