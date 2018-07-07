import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2

Dialog {
    id: updi
    standardButtons: StandardButton.Close | StandardButton.Ignore
    width: 7 * dpi
    height: 5 * dpi

    title: qsTr("Updates")

    onAccepted: { // ignore pressed
        var now = new Date()
        var bumpTime = settings.value("program/versionbump", now.valueOf())
        settings.setValue("program/versionbump", new Date().setDate(now.getDate() + 1).valueOf()) // ensure we only version check once a day
    }

    function display(ignorable) {
        var now = new Date()
        var bumpTime = settings.value("program/versionbump", now.valueOf())
        if ( bumpTime > now.valueOf() ) {
            return; // don't bump more than once a day!
        }

        updi.standardButtons = ignorable ? StandardButton.Close | StandardButton.Ignore : StandardButton.Close
        open()
    }

    TabView {
        id: tabView
        anchors.fill: parent

        Tab {
            title: qsTr("Upgrades")
            anchors.margins: 0.2 * dpi

            Column {
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 0.15 * dpi

                Divider {
                    label: "Etherwall"
                }

                Row {
                    spacing: 0.1 * dpi

                    Label {
                        text: qsTr("Current version:")
                    }

                    TextField {
                        readOnly: true
                        text: Qt.application.version
                    }

                    Label {
                        text: qsTr("Available version:")
                    }

                    TextField {
                        readOnly: true
                        text: transactionModel.latestVersion
                    }
                }

                Button {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    enabled: transactionModel.canUpgrade
                    text: qsTr("Upgrade")
                    // onClicked: // TODO
                }

                Divider {
                    label: nodeManager.nodeName
                }

                Row {
                    spacing: 0.1 * dpi

                    Label {
                        text: qsTr("Current version:")
                    }

                    TextField {
                        readOnly: true
                        text: nodeManager.currentVersion
                    }

                    Label {
                        text: qsTr("Available version:")
                    }

                    TextField {
                        readOnly: true
                        text: nodeManager.latestVersion
                    }
                }

                Button {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    enabled: nodeManager.canUpgrade
                    text: qsTr("Upgrade")
                    onClicked: {
                        nodeManager.upgrade()
                        tabView.currentIndex = 1
                    }
                }
            }
        }

        Tab {
            title: qsTr("Downloads")

            ListView {
                id: listView
                anchors.fill: parent
                anchors.margins: 0.2 * dpi
                model: fileDownloader

                spacing: 0.15 * dpi

                delegate: Row {
                    ProgressBar {
                        anchors.verticalCenter: parent.verticalCenter
                        width: listView.width - abortButton.width - 0.15 * dpi

                        value: file_progress
                        z: 50

                        Text {
                            anchors.centerIn: parent
                            text: file_name
                            z: 100
                        }
                    }

                    Button {
                        id: abortButton
                        enabled: file_running
                        text: file_running ? qsTr("Abort", "file download") : (file_progress < 1.0 ? qsTr("Aborted", "file transfer") : qsTr("Complete", "file download"))
                        anchors.verticalCenter: parent.verticalCenter

                        onClicked: fileDownloader.abort(index)
                    }
                }
            }
        }
    }
}
