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
 * Filter Details dialog
 */

import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Window 2.0

Window {
    id: filterDetails
    title: qsTr("Watch Details")
    signal refresh()

    visible: false
    minimumWidth: 6 * dpi
    minimumHeight: 7 * dpi
    maximumWidth: 10 * dpi
    maximumHeight: 8 * dpi
    width: 7 * dpi
    height: 7 * dpi
    Component.onCompleted: {
        setX(Screen.width / 2.0 - width / 2.0)
        setY(Screen.height / 2.0 - height / 2.0)
    }

    function open( index ) {
        if ( index >= 0 ) {
            nameField.text = filterModel.getName(index)
            contractField.currentIndex = contractModel.getIndex(filterModel.getContract(index))
            var topics = filterModel.getTopics(index)
            eventField.currentIndex = contractModel.getEventIndex(contractField.currentIndex, topics)
            activeField.checked = filterModel.getActive(index)
            topicsField.text = JSON.stringify(topics)
        } else {
            nameField.text = ""
            contractField.currentIndex = -1
            eventField.currentIndex = -1
            activeField.checked = true
            topicsField.text = ""
        }

        show()
    }

    BusyIndicator {
        anchors.centerIn: parent
        z: 10
        running: ipc.starting || ipc.busy || ipc.syncing
    }

    Column {
        id: mainColumn
        anchors.fill: parent
        anchors.margins: 0.1 * dpi
        spacing: 0.2 * dpi

        Row {
            spacing: 0.05 * dpi
            Label {
                width: 1 * dpi
                text: qsTr("Name: ")
            }

            TextField {
                id: nameField
                width: mainColumn.width - 2.3 * dpi

                maximumLength: 255

                onTextChanged: saveButton.refresh()
            }

            Label {
                text: qsTr("Active: ")
                width: 0.7 * dpi
            }

            CheckBox {
                id: activeField
                width: 0.2 * dpi
                checked: true

                onCheckedChanged: saveButton.refresh()
            }
        }

        Row {
            Label {
                width: 1 * dpi
                text: qsTr("Contract: ")
            }

            ComboBox {
                id: contractField
                width: mainColumn.width - 1 * dpi
                model: contractModel
                textRole: "name"

                onCurrentIndexChanged: {
                    eventField.refresh()
                    saveButton.refresh()
                }
            }
        }

        Row {
            Label {
                width: 1 * dpi
                text: qsTr("Event: ")
            }

            ComboBox {
                id: eventField
                width: mainColumn.width - 1 * dpi
                model: contractModel.getEvents(contractField.currentIndex)

                onCurrentTextChanged: {
                    refresh()
                    saveButton.refresh()
                }

                function refresh() {
                    if ( eventField.currentIndex < 0 || contractField.currentIndex < 0 || eventField.currentText.length < 1) {
                        return;
                    }

                    argsView.model = contractModel.getEventArguments(contractField.currentIndex, eventField.currentText, true)
                    argsView.params = []
                    topicsField.text = contractModel.encodeTopics(contractField.currentIndex, eventField.currentText, argsView.params);
                    filterDetails.refresh()
                }
            }
        }

        ListView {
            id: argsView
            width: parent.width
            height: 1.5 * dpi
            property variant params : []

            delegate: Row {
                Label {
                    width: 2.5 * dpi
                    text: modelData.name + "\t" + modelData.type
                }

                ComboBox {
                    id: boolField
                    visible: modelData.type === "bool"
                    width: mainColumn.width - 2.5 * dpi
                    editable: false
                    model: ListModel {
                        ListElement { text: "" }
                        ListElement { text: "true" }
                        ListElement { text: "false" }
                    }

                    Connections {
                        target: filterDetails
                        onRefresh: boolField.currentIndex = 0
                    }

                    onCurrentIndexChanged: {
                        if ( !visible || currentIndex < 0 || contractField.currentIndex < 0 ) return;
                        argsView.params[index] = currentText
                        topicsField.text = contractModel.encodeTopics(contractField.currentIndex, boolField.currentText, argsView.params);
                    }
                }

                TextField {
                    id: valField
                    visible: modelData.type !== "bool"
                    width: mainColumn.width - 2.5 * dpi
                    placeholderText: modelData.placeholder

                    Connections {
                        target: filterDetails
                        onRefresh: valField.text = "" // ensure we wipe old values on window re-open and func reselect
                    }

                    onTextChanged: {
                        if ( !visible || contractField.currentIndex < 0 ) return;
                        argsView.params[index] = text
                        topicsField.text = contractModel.encodeTopics(contractField.currentIndex, eventField.currentText, argsView.params);
                    }

                    validator: RegExpValidator {
                        regExp: modelData.valrex
                    }
                }
            }
        }

        Row {
            Label {
                text: qsTr("Topics: ")
                width: 1 * dpi
            }

            TextArea {
                id: topicsField
                width: mainColumn.width - 1 * dpi
                height: 1.0 * dpi
                wrapMode: Text.WrapAnywhere
                readOnly: true
            }
        }

        Button {
            id: saveButton
            width: parent.width
            height: 1.3 * dpi
            text: "Save"

            Image {
                id: saveIcon
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: parent.height * 0.15
                width: height
                source: "/images/warning"
            }

            style: ButtonStyle {
              label: Text {
                renderType: Text.NativeRendering
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: saveButton.height / 2.0
                text: control.text
              }
            }

            function check() {
                var result = {
                    error: null,
                    name: null,
                    address: null,
                    abi: null
                }

                result.address = contractModel.getAddress(contractField.currentIndex)
                if ( !result.address.match(/0x[a-f,A-Z,0-9]{40}/) ) {
                    result.error = qsTr("Contract address invalid")
                    return result
                }

                result.contract = contractField.currentText

                result.name = nameField.text.trim() || ""
                if ( result.name.length === 0 ) {
                    result.error = qsTr("Invalid filter name")
                    return result
                }

                result.active = activeField.checked

                var topics = topicsField.text.length ? topicsField.text.split(",") : []
                for ( var i = 0; i < topics.length; i++ ) {
                    var topic = topics[i];
                    if ( topic !== "null" && !topic.match(/0x[a-f,A-Z,0-9]{32}/) ) {
                        result.error = qsTr("Filter topic " + topic + " invalid")
                        return result
                    }
                }
                result.topics = topicsField.text

                return result;
            }

            function refresh() {
                var result = check()
                if ( result.error !== null ) {
                    tooltip = result.error
                    saveIcon.source = "/images/warning"
                    return result
                }

                saveIcon.source = "/images/ok"
                return result
            }

            onClicked: {
                var result = refresh()
                if ( result.error !== null ) {
                    errorDialog.text = result.error
                    errorDialog.open()
                    return
                }

                filterModel.addFilter(result.name, result.address, result.contract, result.topics, result.active)
                filterDetails.close()
            }
        }

        Button {
            text: qsTr("Close")
            width: parent.width
            height: 0.6 * dpi

            onClicked: {
                filterDetails.close()
            }
        }

    }
}
