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

import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3 as D
import QtQuick.Window 2.12
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

    title: qsTr("Etherwall Ethereum Wallet") + (ipc.testnet ? " !TESTNET! " : " ") + Qt.application.version + ' [' + ipc.clientVersion + ']'

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

        function onWarning(version, endpoint, warning) {
            if (warning && warning.length) {
                warningDialog.text = warning
                warningDialog.open()
            }
        }
    }

    Connections {
        target: ipc
        function onError() {
            errorDialog.text = ipc.error
            errorDialog.open()
        }
    }

    Connections {
        target: nodeManager
        // onNewNodeVersionAvailable: badge.show("New " + nodeName + " version available. Current: " + curVersion + " Latest: " + newVersion)
        function onError(error) {
            errorDialog.text = error
            errorDialog.open(error)
        }
    }

    // Trezor main connections
    Connections {
        target: trezor
        function onMatrixRequest(type) { pinMatrixDialog.display() }
        function onButtonRequest(code) {
            if ( code !== 8 ) { // tx signing handled in signing windows
                badge.show(badge.button_msg(code))
            }
        }
        function onDeviceOutdated(minVersion, curVersion) {
            errorDialog.text = qsTr("TREZOR firmware outdated")
            errorDialog.text += "\n" + qsTr("Required version", "of firmware") + " " + minVersion
            errorDialog.text += "\n" + qsTr("Current version", "of firmware") + " " + curVersion
            errorDialog.text += "\n" + qsTr("Update firmware at", "url follows") + " https://wallet.trezor.io"
            errorDialog.open()
        }
        function onPassphraseRequest(onDevice) {
            if (onDevice) {
                badge.show(qsTr("Input your password on the TREZOR device"))
            } else {
                trezorPasswordDialog.openFocused()
            }
        }
        function onFailure(error) {
            badge.show("TREZOR: " + error)
        }
        function onError(error) {
            errorDialog.text = "TREZOR critical error: " + error
            errorDialog.open()
        }
    }

    Connections {
        target: transactionModel

        function onLatestVersionChanged(version, manualVersionCheck) {
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
        function onLatestVersionSame(version, manualVersionCheck) {
            if ( !manualVersionCheck ) {
                return
            }

            manualVersionCheck = false
            versionDialog.title = qsTr("Etherwall up to date")
            versionDialog.text = qsTr("Etherwall is up to date")
            versionDialog.detailedText = "Current version: " + Qt.application.version + "\nLatest stable version: " + transactionModel.latestVersion
            versionDialog.open()
        }

        function onReceivedTransaction(toAddress) { badge.show(qsTr("Received a new transaction to: ") + toAddress) }
        function onConfirmedTransaction(fromAddress, toAddress, hash) {
            if ( toAddress.length ) {
                badge.show(qsTr("Confirmed transaction to: ") + toAddress)
            } else { // contract creation
                ipc.getTransactionReceipt(hash)
            }
        }
    }

    Connections {
        target: eventModel

        function onReceivedEvent(contract, signature) {
            badge.show(qsTr("Received event from contract " + contract + ": ") + signature)
        }
    }

    Connections {
        target: contractModel

        function onReceivedTokens(value, token, sender) {
            badge.show(qsTr("Received") + " " + value + " " + token + " " + qsTr("from") + " " + sender)
        }
    }

    Connections {
        target: accountModel

        function onWalletExportedEvent() { badge.show(qsTr("Wallet successfully exported")) }
        function onWalletImportedEvent() { badge.show(qsTr("Wallet succesfully imported")) }
        function onWalletErrorEvent() { badge.show(qsTr("Error on wallet import/export: " + error)) }
        function onPromptForTrezorImport() { trezorImportDialog.display(qsTr("Detected TREZOR device with unimported accounts. Import addresses from TREZOR?"), "https://www.etherwall.com/faq/#importaccount") }
        function onAccountsRemoved() { badge.show(qsTr("TREZOR accounts removed")) }
    }

    Connections {
        target: ipc

        function onGetTransactionReceiptDone(receipt) {
            var cname = contractModel.contractDeployed(receipt)
            if ( cname.length ) {
                badge.show(qsTr("Contract ") + cname + qsTr(" succesfully deployed: " + receipt.contractAddress))
            }
        }
    }

    D.FileDialog {
        id: exportFileDialog
        title: qsTr("Export wallet backup")
        selectFolder: false
        selectExisting: false
        selectMultiple: false
        folder: shortcuts.documents
        nameFilters: [ "Etherwall backup files (*.etherwall)", "All files (*)" ]

        onAccepted: accountModel.exportWallet(exportFileDialog.fileUrl)
    }

    D.FileDialog {
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

    Dialog {
        id: aboutDialog
        anchors.centerIn: parent

        // icon: D.StandardIcon.Information
        width: 7 * dpi
        title: qsTr("About Etherwall")
        standardButtons: Dialog.Ok | Dialog.Help

        Column {
            Text {
                text: 'Etherwall ' + Qt.application.version + ' copyright 2015-2020 by Aleš Katona.'
            }

            Text {
                text: qsTr("Etherwall version: ", "about details") + Qt.application.version
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }

            Text {
                text: "Geth version: " + ipc.clientVersion
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }

            Text {
                text: "Chain: " + (ipc.testnet ? "Rinkeby" : "Homestead")
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }

            Text {
                text: "Node Endpoint: " + ipc.endpoint
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
        }

        onHelpRequested: Qt.openUrlExternally("https://www.etherwall.com")
    }

    TrezorImportDialog {
        id: trezorImportDialog
    }

    Dialog {
        id: errorDialog
        anchors.centerIn: parent
        property string text : qsTr("Unknown Error")

        standardButtons: Dialog.Ok
        // modality: Qt.ApplicationModal
        // icon: D.StandardIcon.Critical
        width: 5 * dpi


        Text {
            text: errorDialog.text
        }
    }

    Dialog {
        id: warningDialog
        anchors.centerIn: parent

        property string text : ""
        // icon: D.StandardIcon.Warning
        width: 5 * dpi
        title: qsTr("Warning")
        Text {
            text: warningDialog.text
        }
        standardButtons: Dialog.Ok

        onAccepted: initializer.proceed()
    }

    Dialog {
        id: versionDialog
        anchors.centerIn: parent

        property string text : ""
        property string detailedText : ""

        standardButtons: Dialog.Ok

        Column {
            Text {
                text: versionDialog.text
            }

            Text {
                text: versionDialog.detailedText
            }
        }

        // icon: D.StandardIcon.Information
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

    BusyIndicatorFixed {
        anchors.centerIn: parent
        z: 10
        visible: true  // TODO bugged in wayland it seems
        running: ipc.starting || ipc.busy || ipc.syncing || accountModel.busy || trezor.busy
    }

    FirstTimeDialog {
        visible: !settings.contains("program/v2firstrun")
        onRejected: closeTimer.start()
    }

    Item {
        anchors.fill: parent

        TabBar {
            id: tabView
            anchors.left: parent.left
            anchors.right: parent.right

            TabButton {
                text: qsTr("Accounts")
            }
            TabButton {
                text: qsTr("Transactions")
            }
            TabButton {
                text: qsTr("Contracts")
            }
            TabButton {
                text: qsTr("Currencies")
            }
            TabButton {
                text: qsTr("Settings")
            }
            TabButton {
                text: qsTr("Logs")
            }
        }

        StackLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: tabView.bottom
            anchors.bottom: parent.bottom

            currentIndex: tabView.currentIndex

            AccountsTab {}
            TransactionsTab {}
            ContractsTab {}
            CurrencyTab {}
            SettingsTab {}
            InfoTab {}
        }
    }

    footer: ToolBar {
        height: 38
        enabled: !ipc.busy

        Row {
            id: leftButtonsRow

            ToolButton {
                id: blockButton
                height: 32
                width: 32
                enabled: parent.enabled && (ipc.connectionState > 0)
                icon.source: "/images/block"
                icon.color: "transparent"
                hoverEnabled: true
                ToolTip.text: qsTr("Block number: ") + transactionModel.blockNumber
                ToolTip.visible: hovered

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
                icon.source: "/images/gas"
                icon.color: "transparent"
                ToolTip.text: qsTr("Gas price: ") + transactionModel.gasPrice
                ToolTip.visible: hovered
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
                icon.source: "/images/trezor"
                // icon.color: trezor.ini ? "gray" : "transparent"
                enabled: trezor.initialized
                height: 32
                width: 32
                ToolTip.text: "TREZOR: " + (trezor.initialized ? qsTr("initialized") : (trezor.present ? qsTr("present") : qsTr("disconnected")))
                ToolTip.visible: hovered
                onClicked: badge.show("TREZOR " + qsTr("device id: ", "trezor") + trezor.deviceID)
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

                icon.source: "/images/connected" + getQuality(ipc.connectionState, ipc.peerCount)
                icon.color: "transparent"
                height: 32
                width: 32
                enabled: !ipc.starting
                ToolTip.text: qsTr("Connection state: ") + connectionState(ipc.connectionState, ipc.peerCount)
                ToolTip.visible: hovered
                onClicked: {
                    badge.show(qsTr("Connection state: ") + connectionState(ipc.connectionState, ipc.peerCount))
                }
            }
        }
    }

}
