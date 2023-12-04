import QtQuick 2.15
import QtQuick.Controls 2.15

import ThemeEngine 1.0

Item {
    id: screenDeviceList
    anchors.fill: parent

    ////////////////////////////////////////////////////////////////////////////

    Component.onCompleted: { checkBluetoothStatus() }

    Connections {
        target: deviceManager
        function onBluetoothChanged() { checkBluetoothStatus() }
    }

    function loadScreen() {
        checkBluetoothStatus()

        // change screen
        appContent.state = "DeviceList"
    }

    function checkBluetoothStatus() {

        //console.log(">> deviceManager.bluetooth : " + deviceManager.bluetooth)
        //console.log(">> deviceManager.bluetoothAdapter : " + deviceManager.bluetoothAdapter)
        //console.log(">> deviceManager.bluetoothEnabled : " + deviceManager.bluetoothEnabled)
        //console.log(">> deviceManager.bluetoothPermissions : " + deviceManager.bluetoothPermissions)

        //console.log(">> deviceManager.permissionOS : " + deviceManager.permissionOS)
        //console.log(">> deviceManager.permissionLocationBLE : " + deviceManager.permissionLocationBLE)
        //console.log(">> deviceManager.permissionLocationBackground : " + deviceManager.permissionLocationBackground)
        //console.log(">> deviceManager.permissionLocationGPS : " + deviceManager.permissionLocationGPS)

        if (deviceManager.hasDevices) {
            // The device list is shown
            itemStatus.source = ""

            if (!deviceManager.bluetoothPermissions) {
                rectangleBluetoothStatus.setPermissionWarning()
            } else if (!deviceManager.bluetoothAdapter) {
                rectangleBluetoothStatus.setAdapterWarning()
            } else if (!deviceManager.bluetoothEnabled) {
                rectangleBluetoothStatus.setBluetoothWarning()
            } else {
                rectangleBluetoothStatus.hide()
            }
        } else {
            // The device list is not populated
            rectangleBluetoothStatus.hide()

            if (!deviceManager.bluetoothPermissions) {
                itemStatus.source = "ItemNoPermissions.qml"
            } else if (!deviceManager.bluetoothAdapter || !deviceManager.bluetoothEnabled) {
                itemStatus.source = "ItemNoBluetooth.qml"
            } else {
                itemStatus.source = "ItemNoDevice.qml"
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    property bool selectionMode: false
    property var selectionList: []
    property int selectionCount: 0

    function selectDevice(index) {
        // make sure it's not already selected
        if (deviceManager.getDeviceByProxyIndex(index).selected) return

        // then add
        selectionMode = true
        selectionList.push(index)
        selectionCount++

        deviceManager.getDeviceByProxyIndex(index).selected = true
    }
    function deselectDevice(index) {
        var i = selectionList.indexOf(index)
        if (i > -1) { selectionList.splice(i, 1); selectionCount--; }
        if (selectionList.length <= 0 || selectionCount <= 0) { exitSelectionMode() }

        deviceManager.getDeviceByProxyIndex(index).selected = false
    }

    function isSelected() {
        return (selectionList.length > 0)
    }
    function exitSelectionMode() {
        selectionMode = false
        selectionList = []
        selectionCount = 0

        for (var i = 0; i < devicesView.count; i++) {
            deviceManager.getDeviceByProxyIndex(i).selected = false
        }
    }

    function updateSelectedDevice() {
        for (var i = 0; i < devicesView.count; i++) {
            if (deviceManager.getDeviceByProxyIndex(i).selected) {
                deviceManager.updateDevice(deviceManager.getDeviceByProxyIndex(i).deviceAddress)
            }
        }
        exitSelectionMode()
    }
    function syncSelectedDevice() {
        for (var i = 0; i < devicesView.count; i++) {
            if (deviceManager.getDeviceByProxyIndex(i).selected) {
                deviceManager.syncDevice(deviceManager.getDeviceByProxyIndex(i).deviceAddress)
            }
        }
        exitSelectionMode()
    }
    function removeSelectedDevice() {
        var devicesAddr = []
        for (var i = 0; i < devicesView.count; i++) {
            if (deviceManager.getDeviceByProxyIndex(i).selected) {
                devicesAddr.push(deviceManager.getDeviceByProxyIndex(i).deviceAddress)
            }
        }
        for (var count = 0; count < devicesAddr.length; count++) {
            deviceManager.removeDevice(devicesAddr[count])
        }
        exitSelectionMode()
    }

    ////////////////////////////////////////////////////////////////////////////

    PopupDeleteDevice {
        id: confirmDeleteDevice
        onConfirmed: screenDeviceList.removeSelectedDevice()
    }

    ////////////////////////////////////////////////////////////////////////////

    Column {
        id: rowbar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        z: 2

        ////////////////

        Rectangle {
            id: rectangleBluetoothStatus
            anchors.left: parent.left
            anchors.right: parent.right

            height: 0
            Behavior on height { NumberAnimation { duration: 133 } }

            clip: true
            visible: (height > 0)
            color: Theme.colorActionbar

            // prevent clicks below this area
            MouseArea { anchors.fill: parent; acceptedButtons: Qt.AllButtons; }

            Text {
                id: textBluetoothStatus
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16

                color: Theme.colorActionbarContent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                font.bold: isDesktop ? true : false
                font.pixelSize: Theme.componentFontSize
            }

            ButtonWireframe {
                id: buttonBluetoothStatus
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                height: 32

                fullColor: true
                primaryColor: Theme.colorActionbarHighlight

                text: {
                    if (!deviceManager.bluetoothPermissions) return qsTr("Request")
                    if (Qt.platform.os === "android") {
                        if (!deviceManager.bluetoothEnabled) return qsTr("Enable")
                    }
                    return qsTr("Retry")
                }
                onClicked: {
                    if (!deviceManager.bluetoothPermissions) {
                        deviceManager.requestBluetoothPermissions()
                    }
                    if (!deviceManager.bluetoothEnabled) {
                        deviceManager.enableBluetooth()
                    }
                    deviceManager.checkBluetooth()
                }
            }

            function hide() {
                rectangleBluetoothStatus.height = 0
            }
            function setAdapterWarning() {
                textBluetoothStatus.text = qsTr("Bluetooth adapter not found...")
                rectangleBluetoothStatus.height = 48
            }
            function setBluetoothWarning() {
                textBluetoothStatus.text = qsTr("Bluetooth is disabled...")
                rectangleBluetoothStatus.height = 48
            }
            function setPermissionWarning() {
                textBluetoothStatus.text = qsTr("Bluetooth permission missing...")
                rectangleBluetoothStatus.height = 48
            }
        }

        ////////////////

        Rectangle {
            id: rectangleActions
            anchors.left: parent.left
            anchors.right: parent.right

            height: (screenDeviceList.selectionCount) ? 48 : 0
            Behavior on height { NumberAnimation { duration: 133 } }

            clip: true
            visible: (height > 0)
            color: Theme.colorActionbar

            // prevent clicks below this area
            MouseArea { anchors.fill: parent; acceptedButtons: Qt.AllButtons; }

            Row {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                RoundButtonIcon {
                    id: buttonClear
                    width: 36
                    height: 36
                    anchors.verticalCenter: parent.verticalCenter

                    source: "qrc:/assets/icons_material/baseline-backspace-24px.svg"
                    rotation: 180
                    iconColor: Theme.colorActionbarContent
                    backgroundColor: Theme.colorActionbarHighlight
                    onClicked: screenDeviceList.exitSelectionMode()
                }

                Text {
                    id: textActions
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTr("%n device(s) selected", "", screenDeviceList.selectionCount)
                    color: Theme.colorActionbarContent
                    font.bold: true
                    font.pixelSize: Theme.componentFontSize
                }
            }

            Row {
                anchors.right: parent.right
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                ButtonCompactable {
                    id: buttonDelete
                    height: compact ? 36 : 34
                    anchors.verticalCenter: parent.verticalCenter

                    compact: !wideMode
                    iconColor: Theme.colorActionbarContent
                    backgroundColor: Theme.colorActionbarHighlight
                    onClicked: confirmDeleteDevice.open()

                    text: qsTr("Delete")
                    source: "qrc:/assets/icons_material/baseline-delete-24px.svg"
                }

                ButtonCompactable {
                    id: buttonRefresh
                    height: !wideMode ? 36 : 34
                    anchors.verticalCenter: parent.verticalCenter
                    visible: deviceManager.bluetooth

                    compact: !wideMode
                    iconColor: Theme.colorActionbarContent
                    backgroundColor: Theme.colorActionbarHighlight
                    onClicked: screenDeviceList.updateSelectedDevice()

                    text: qsTr("Refresh")
                    source: "qrc:/assets/icons_material/baseline-refresh-24px.svg"
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    GridView {
        id: devicesView

        anchors.top: rowbar.bottom
        anchors.topMargin: singleColumn ? 0 : 8
        anchors.left: screenDeviceList.left
        anchors.leftMargin: 6
        anchors.right: screenDeviceList.right
        anchors.rightMargin: 6
        anchors.bottom: screenDeviceList.bottom
        anchors.bottomMargin: singleColumn ? 0 : 8

        property bool bigWidget: (!isHdpi || (isTablet && width >= 480))

        property int cellWidthTarget: {
            if (singleColumn) return devicesView.width
            if (isTablet) return (bigWidget ? 350 : 280)
            return (bigWidget ? 440 : 320)
        }
        property int cellColumnsTarget: Math.trunc(devicesView.width / cellWidthTarget)

        cellWidth: (devicesView.width / cellColumnsTarget)
        cellHeight: (bigWidget ? 144 : 100)

        ScrollBar.vertical: ScrollBar {
            visible: false
            anchors.right: parent.right
            anchors.rightMargin: -6
            policy: ScrollBar.AsNeeded
        }

        model: deviceManager.devicesList
        delegate: DeviceWidget {
            width: devicesView.cellWidth
            height: devicesView.cellHeight
            bigAssMode: devicesView.bigWidget
            singleColumn: (singleColumn || devicesView.cellColumnsTarget === 1)
        }
    }

    ////////

    Loader {
        id: itemStatus
        anchors.fill: parent
        visible: !deviceManager.hasDevices
        asynchronous: true
    }

    ////////

    Row {
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        spacing: 12

        visible: isDesktop

        ButtonWireframe {
            text: qsTr("temps")
            fullColor: true
            primaryColor: Theme.colorSecondary
            onClicked: screenTemperaturePresetList.loadScreen()
        }

        ButtonWireframe {
            text: qsTr("devices")
            fullColor: true
            primaryColor: Theme.colorSecondary
            onClicked: screenDeviceBrowser.loadScreen()
        }
    }

    ////////
}
