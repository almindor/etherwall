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

    id: appWindow
    visible: true
    minimumWidth: 8 * dpi
    minimumHeight: 6 * dpi

    width: settings.value("program/window/width", 10 * dpi)
    height: settings.value("program/window/height", 7 * dpi)

    onWidthChanged: {
        settings.setValue("program/window/width", width)
    }

    onHeightChanged: {
        settings.setValue("program/window/height", height)
    }

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

    PasswordDialog {
        id: trezorPasswordDialog
        title: qsTr("TREZOR passphrase")
        text: qsTr("Please provide your TREZOR passphrase")

        onPasswordSubmitted: trezor.submitPassphrase(password)
        onPasswordRejected: trezor.cancel()
    }

    Connections {
        target: initializer

        onWarning: {
            if (warning && warning.length) {
                warningDialog.text = warning
                warningDialog.open()
            }
        }
    }

    Connections {
        target: ipc
        onError: {
            errorDialog.text = ipc.error
            errorDialog.open()
        }
    }

    Connections {
        target: nodeManager
        onNewNodeVersionAvailable: badge.show("New " + nodeName + " version available. Current: " + curVersion + " Latest: " + newVersion)
        onError: {
            errorDialog.text = error
            errorDialog.open(error)
        }
    }

    // Trezor main connections
    Connections {
        target: trezor
        onMatrixRequest: pinMatrixDialog.display()
        onButtonRequest: {
            if ( code != 8 ) { // tx signing handled in signing windows
                badge.show(badge.button_msg(code))
            }
        }
        onPassphraseRequest: trezorPasswordDialog.openFocused()
        onFailure: {
            errorDialog.text = "TREZOR: " + error
            errorDialog.open()
        }
        onError: {
            log.log(error, 3)
            errorDialog.text = "TREZOR critical error: " + error
            errorDialog.open()
        }
    }

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
            versionDialog.text = qsTr("New version of Etherwall available: ") + transactionModel.latestVersion
            versionDialog.open()
        }
        onLatestVersionSame: {
            if ( !manualVersionCheck ) {
                return
            }

            manualVersionCheck = false
            versionDialog.title = qsTr("Etherwall up to date")
            versionDialog.text = qsTr("Etherwall is up to date")
            versionDialog.detailedText = "Current version: " + Qt.application.version + "\nLatest stable version: " + transactionModel.latestVersion
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
        target: contractModel

        onReceivedTokens: badge.show(qsTr("Received") + " " + value + " " + token + " " + qsTr("from") + " " + sender)
    }

    Connections {
        target: accountModel

        onWalletExportedEvent: badge.show(qsTr("Wallet successfully exported"))
        onWalletImportedEvent: badge.show(qsTr("Wallet succesfully imported"))
        onWalletErrorEvent: badge.show(qsTr("Error on wallet import/export: " + error))
        onPromptForTrezorImport: trezorImportDialog.display(qsTr("Detected TREZOR device with unimported accounts. Import addresses from TREZOR?"), "https://www.etherwall.com/faq/#importaccount")
        onAccountsRemoved: badge.show(qsTr("TREZOR accounts removed"))
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
                text: qsTr("About", "etherwall")
                onTriggered: aboutDialog.open()
            }

            MenuItem {
                text: qsTr("Check for updates", "in main menu")
                onTriggered: transactionModel.checkVersion(true)
            }

            MenuItem {
                text: qsTr("Export", "wallet")
                onTriggered: exportFileDialog.open()
            }

            MenuItem {
                text: qsTr("Import", "wallet")
                onTriggered: importFileDialog.open()
            }

            MenuItem {
                text: qsTr("Quit")
                onTriggered: appWindow.close()
            }
        }
    }

    MessageDialog {
        id: aboutDialog
        icon: StandardIcon.Information
        width: 5 * dpi
        title: qsTr("About Etherwall")
        standardButtons: StandardButton.Ok | StandardButton.Help
        text: 'Etherwall ' + Qt.application.version + ' copyright 2015-2018 by Aleš Katona.'
        detailedText: qsTr("Etherwall version: ", "about details") + Qt.application.version + "\nGeth version: " + ipc.clientVersion + "\n" + (ipc.testnet ? "Running on testnet (rinkeby)" : "")
        onHelp: Qt.openUrlExternally("https://www.etherwall.com")
    }

    TrezorImportDialog {
        id: trezorImportDialog
    }

    MessageDialog {
        id: errorDialog
        modality: Qt.ApplicationModal
        icon: StandardIcon.Critical
        width: 5 * dpi
    }

    MessageDialog {
        id: warningDialog
        icon: StandardIcon.Warning
        width: 5 * dpi
        title: qsTr("Warning")

        onAccepted: initializer.proceed()
    }

    MessageDialog {
        id: versionDialog
        icon: StandardIcon.Information
        width: 5 * dpi
        title: qsTr("New version available")
    }

    function showBadge(val) {
        badge.show(val)
    }

    Badge {
        id: badge
        z: 999
    }

    BusyIndicator {
        anchors.centerIn: parent
        z: 10
        running: ipc.starting || ipc.busy || ipc.syncing || accountModel.busy || trezor.busy
    }

    FirstTimeDialog {
        visible: !settings.contains("program/v2firstrun")
        onRejected: closeTimer.start()
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

    MessageDialog {
        id: trezorDialog
        title: "TREZOR"
        text: "TREZOR " + qsTr("device id: ", "trezor") + trezor.deviceID
        icon: StandardIcon.Information
        standardButtons: StandardButton.Ok
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
            text: ipc.closing ? qsTr("Closing app") : (ipc.starting ? qsTr("Connecting to node...") : (ipc.busy ? (qsTr("Processing ") + ipc.activeRequestName) : qsTr("Ready")))
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
                    badge.show(qsTr("Connection state: ") + connectionState(ipc.connectionState, ipc.peerCount))
                }
            }
        }
    }

}
