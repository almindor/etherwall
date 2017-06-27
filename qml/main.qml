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
/** @file main.qml
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Main app window
 */

import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.0
import "components"

ApplicationWindow {
    property int dpi: Screen.pixelDensity * 25.4;
    property bool manualVersionCheck: false

    id: appWindow
    visible: true
    minimumWidth: 8 * dpi
    minimumHeight: 6 * dpi

    width: 8 * dpi
    height: 6 * dpi

    Component.onCompleted: {
        setX(Screen.width / 2.0 - width / 2.0)
        setY(Screen.height / 2.0 - height / 2.0)
    }

    title: qsTr("Etherdyne Ethereum Wallet") + (ipc.testnet ? " !TESTNET! " : " ") + Qt.application.version + ' [' + ipc.clientVersion + ']'

    Timer {
        id: closeTimer
        interval: 100
        running: false

        onTriggered: {
            appWindow.close()
        }
    }

    onClosing: {
        close.accepted = ipc.closeApp()

        if ( !close.accepted && !closeTimer.running ) {
            closeTimer.start()
        }
    }

    PinMatrixDialog {
        id: pinMatrixDialog
        // visible: true
    }

    ButtonRequestDialog {
        id: buttonRequestDialog
    }

    PasswordDialog {
        id: trezorPasswordDialog
        title: qsTr("TREZOR passphrase")
        msg: qsTr("Please provide your TREZOR passphrase")

        onAccepted: {
            trezor.submitPassphrase(password)
        }

        onRejected: {
            trezor.cancel()
        }
    }

    // Trezor main connections
    Connections {
        target: trezor
        onMatrixRequest: pinMatrixDialog.open()
        onButtonRequest: {
            if ( code != 8 ) { // tx signing handled in signing windows
                badge.show(badge.button_msg(code))
            }
        }
        onPassphraseRequest: trezorPasswordDialog.open()
        onFailure: {
            errorDialog.msg = "TREZOR: " + error
            errorDialog.open()
        }
        onError: {
            log.log(error, 3)
            errorDialog.msg = "TREZOR critical error: " + error
            errorDialog.open()
        }
    }

    FileDialog {
        id: exportFileDialog
        title: qsTr("Export wallet backup")
        selectFolder: false
        selectExisting: false
        selectMultiple: false
        folder: shortcuts.documents
        nameFilters: [ "Etherwall backup files (*.etherwall)", "All files (*)" ]

        onAccepted: accountModel.exportWallet(exportFileDialog.fileUrl)
    }

    FileDialog {
        id: importFileDialog
        title: qsTr("Import wallet from backup")
        selectFolder: false
        selectExisting: true
        selectMultiple: false
        folder: shortcuts.documents
        nameFilters: [ "Etherwall backup files (*.etherwall)", "All files (*)" ]

        onAccepted: accountModel.importWallet(importFileDialog.fileUrl)
    }

    menuBar: MenuBar {
        Menu {
            title: "Wallet"

            MenuItem {
                text: "About"
                onTriggered: aboutDialog.open()
            }

            MenuItem {
                text: "Export"
                onTriggered: exportFileDialog.open()
            }

            MenuItem {
                text: "Import"
                onTriggered: importFileDialog.open()
            }
        }
    }

    ConfirmDialog {
        id: aboutDialog
        width: 5 * dpi
        title: qsTr("About Etherwall")
        yesText: qsTr("Check for updates")
        noText: qsTr("OK")
        msg: '<html><body>Etherwall ' + Qt.application.version + ' copyright 2015-2017 by Aleš Katona. For more info please visit the <a href="http://etherwall.com">homepage</a></body></html>'
        onYes: {
            manualVersionCheck = true
            transactionModel.checkVersion()
        }
    }

    TrezorImportDialog {
        id: trezorImportDialog
        width: 5 * dpi
        yesText: qsTr("Import")
        noText: qsTr("Cancel")
    }

    ErrorDialog {
        id: errorDialog
        width: 5 * dpi

        Connections {
            target: ipc
            onError: {
                errorDialog.msg = ipc.error
                errorDialog.open()
            }
        }
    }

    ErrorDialog {
        id: versionDialog
        width: 5 * dpi
    }

    function showBadge(val) {
        badge.show(val)
    }

    Badge {
        id: badge
        z: 999

        Connections {
            target: transactionModel

            onLatestVersionChanged: {
                if ( !manualVersionCheck ) {
                    var now = new Date()
                    var bumpTime = settings.value("program/versionbump", now.valueOf())
                    if ( bumpTime > now.valueOf() ) {
                        return; // don't bump more than once a day!
                    }
                    settings.setValue("program/versionbump", new Date().setDate(now.getDate() + 1).valueOf())
                }

                manualVersionCheck = false
                versionDialog.title = qsTr("Update available")
                versionDialog.msg = qsTr("New version of Etherwall available: ") + transactionModel.latestVersion
                versionDialog.open()
            }
            onLatestVersionSame: {
                if ( !manualVersionCheck ) {
                    return
                }

                manualVersionCheck = false
                versionDialog.title = qsTr("Etherwall up to date")
                versionDialog.msg = qsTr("Etherwall is up to date: ") + transactionModel.latestVersion
                versionDialog.open()
            }

            onReceivedTransaction: badge.show(qsTr("Received a new transaction to: ") + toAddress)
            onConfirmedTransaction: {
                if ( toAddress.length ) {
                    badge.show(qsTr("Confirmed transaction to: ") + toAddress)
                } else { // contract creation
                    ipc.getTransactionReceipt(hash)
                }
            }
        }

        Connections {
            target: eventModel

            onReceivedEvent: badge.show(qsTr("Received event from contract " + contract + ": ") + signature)
        }

        Connections {
            target: accountModel

            onWalletExportedEvent: badge.show(qsTr("Wallet successfully exported"))
            onWalletImportedEvent: badge.show(qsTr("Wallet succesfully imported"))
            onWalletErrorEvent: badge.show(qsTr("Error on wallet import/export: " + error))
            onPromptForTrezorImport: trezorImportDialog.open(qsTr("Detected TREZOR device with unimported accounts.") + '<br><a href="http://www.etherwall.com/faq/#importaccount">' + qsTr("Import addresses from TREZOR?") + '</a>')
        }

        Connections {
            target: ipc

            onGetTransactionReceiptDone: {
                var cname = contractModel.contractDeployed(receipt)
                if ( cname.length ) {
                    badge.show(qsTr("Contract ") + cname + qsTr(" succesfully deployed: " + receipt.contractAddress))
                }
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        z: 10
        running: ipc.starting || ipc.busy || ipc.syncing || accountModel.busy || trezor.busy
    }

    FirstTimeDialog {
        visible: !settings.contains("program/v2firstrun")
    }

    TabView {
        id: tabView
        anchors.fill: parent

        AccountsTab {}

        TransactionsTab {}

        ContractsTab {}

        CurrencyTab {}

        SettingsTab {}

        InfoTab {}
    }

    BaseDialog {
        id: trezorDialog
    }

    statusBar: StatusBar {
        height: 38
        enabled: !ipc.busy

        Row {
            id: leftButtonsRow
            ToolButton {
                id: blockButton
                height: 32
                width: 32
                enabled: parent.enabled && (ipc.connectionState > 0)
                iconSource: "/images/block"
                tooltip: qsTr("Block number: ") + transactionModel.blockNumber
                onClicked: {
                    blockField.visible = !blockField.visible

                    if ( blockField.visible ) {
                        blockField.selectAll()
                        blockField.copy()
                    }
                }
            }

            TextField {
                id: blockField
                visible: false
                width: 100
                readOnly: true
                text: transactionModel.blockNumber
            }

            ToolButton {
                id: gasButton
                height: 32
                width: 32
                enabled: parent.enabled && (ipc.connectionState > 0)
                iconSource: "/images/gas"
                tooltip: qsTr("Gas price: ") + transactionModel.gasPrice
                onClicked: {
                    gasField.visible = !gasField.visible

                    if ( gasField.visible ) {
                        gasField.selectAll()
                        gasField.copy()
                    }
                }
            }

            TextField {
                id: gasField
                visible: false
                width: 200
                readOnly: true
                text: transactionModel.gasPrice
            }
        }

        Text {
            anchors {
                verticalCenter: parent.verticalCenter
                left: leftButtonsRow.right
                right: rightButtonsRow.left
            }

            visible: !ipc.syncing
            horizontalAlignment: Text.AlignHCenter
            text: ipc.closing ? qsTr("Closing app") : (ipc.starting ? qsTr("Starting Geth...") : qsTr("Ready"))
        }

        Text {
            anchors {
                verticalCenter: parent.verticalCenter
                left: leftButtonsRow.right
                right: rightButtonsRow.left
            }

            visible: ipc.syncing
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Synchronized ") + Math.max(0, ipc.currentBlock - ipc.startingBlock) + qsTr(" out of ") + Math.max(0, ipc.highestBlock - ipc.startingBlock) + qsTr(" blocks")
        }

        ProgressBar {
            anchors.leftMargin: 0.05 * dpi
            anchors.rightMargin: 0.05 * dpi
            anchors.left: leftButtonsRow.right
            anchors.right: rightButtonsRow.left
            anchors.verticalCenter: parent.verticalCenter
            z: -999
            visible: ipc.syncing
            value: Math.max(0, ipc.currentBlock - ipc.startingBlock) / Math.max(1, ipc.highestBlock - ipc.startingBlock)
        }

        Row {
            id: rightButtonsRow
            anchors.right: parent.right

            ToolButton {
                iconSource: "/images/trezor"
                height: 32
                width: 32
                enabled: trezor.initialized
                tooltip: "TREZOR: " + (trezor.initialized ? qsTr("initialized") : (trezor.present ? qsTr("present") : qsTr("disconnected")))
                onClicked: {
                    trezorDialog.title = "TREZOR"
                    trezorDialog.msg = "TREZOR " + qsTr("device id: ") + trezor.deviceID
                    trezorDialog.open()
                }
            }

            ToolButton {
                function getQuality(cs, pc) {
                    if ( ipc.thinClient ) {
                        return 3
                    }

                    if ( cs <= 0 ) {
                        return 0 // disconnected
                    }

                    if ( pc > 6 ) {
                        return 3 // high
                    } else if ( pc > 3 ) {
                        return 2 // medium
                    } else {
                        return 1 // low
                    }
                }

                function connectionState(cs, pc) {
                    if ( cs <= 0 ) {
                        return qsTr("disconnected", "connection state")
                    }

                    if ( ipc.thinClient ) {
                        return qsTr("connected to remote node", "connection state")
                    }

                    return qsTr("connected with ", "connection state connected with X peers") + ipc.peerCount + qsTr(" peers", "connection status, peercount")
                }

                iconSource: "/images/connected" + getQuality(ipc.connectionState, ipc.peerCount)
                height: 32
                width: 32
                enabled: !ipc.starting
                tooltip: qsTr("Connection state: ") + connectionState(ipc.connectionState, ipc.peerCount)
                onClicked: {
                    ipc.connectToServer()
                }
            }
        }
    }

}
