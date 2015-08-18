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
    signal accepted
    signal yes

    property string msg

    modality: Qt.ApplicationModal
    visible: false
    width: 600
    height: 70

    function open() {
        visible = true;
    }

    function close() {
        visible = false;
    }

    Label {
        y: 10
        x: 10
        text: msg
        wrapMode: Text.Wrap
        width: parent.width

        Keys.onEscapePressed: {
            close()
        }
    }
}
