import QtQuick
import QtCharts

import ThemeEngine
import "qrc:/js/UtilsNumber.js" as UtilsNumber

Item {
    id: chartProbeRealTime
    anchors.fill: parent
    anchors.margins: 12

    property bool useOpenGL: false
    property bool showGraphDots: false
    property color legendColor: Theme.colorSubText

    property real valueMin: 0
    property real valueMax: 100

    ////////////////////////////////////////////////////////////////////////////

    Connections {
        target: currentDevice
        function onRtGraphUpdated() {
            console.log("onRtgraphUpdated")

            currentDevice.updateRtGraph(axisTime, 5,
                                        temp1Data, temp2Data, temp3Data,
                                        temp4Data, temp5Data, temp6Data)
        }
        function onRtGraphCleaned() {
            console.log("onRtgraphCleaned")

            currentDevice.updateRtGraph(axisTime, 5,
                                        temp1Data, temp2Data, temp3Data,
                                        temp4Data, temp5Data, temp6Data)
        }
    }

    function loadGraph() {
        if (typeof currentDevice === "undefined" || !currentDevice) return
        //console.log("chartProbeRealTime // loadGraph() >> " + currentDevice)

        temp1Data.visible = currentDevice.hasTemperature1
        temp2Data.visible = currentDevice.hasTemperature2
        temp3Data.visible = currentDevice.hasTemperature3
        temp4Data.visible = currentDevice.hasTemperature4
        temp5Data.visible = currentDevice.hasTemperature5
        temp6Data.visible = currentDevice.hasTemperature6

        currentDevice.updateRtGraph(axisTime, 5,
                                    temp1Data, temp2Data, temp3Data,
                                    temp4Data, temp5Data, temp6Data)

        axisTemp.min = 0
        axisTemp.max = 100

        legendColor = Qt.rgba(legendColor.r, legendColor.g, legendColor.b, 0.8)
    }

    function updateGraph() {
        if (typeof currentDevice === "undefined" || !currentDevice) return
        //console.log("chartProbeRealTime // updateGraph() >> " + currentDevice)
    }

    function isIndicator() { return false }
    function resetIndicator() { }

    ////////////////////////////////////////////////////////////////////////////

    Rectangle {
        id: dataIndicator

        width: dataIndicatorContent.width + 24
        height: 32
        z: 2

        radius: 4
        color: Theme.colorForeground
        border.width: Theme.componentBorderWidth
        border.color: Theme.colorSeparator

        Row {
            id: dataIndicatorContent
            anchors.centerIn: parent
            spacing: 16

            Rectangle { // #1
                anchors.verticalCenter: parent.verticalCenter
                width: 20; height: 20; radius: 20;

                visible: currentDevice.hasTemperature1
                color: temp1Data.color
                Text {
                    anchors.centerIn: parent
                    text: "1"
                    color: "white"
                    font.pixelSize: 15
                    font.bold: true
                }
            }

            Rectangle { // #2
                anchors.verticalCenter: parent.verticalCenter
                width: 20; height: 20; radius: 20;

                visible: currentDevice.hasTemperature2
                color: temp2Data.color
                Text {
                    anchors.centerIn: parent
                    text: "2"
                    color: "white"
                    font.pixelSize: 15
                    font.bold: true
                }
            }

            Rectangle { // #3
                anchors.verticalCenter: parent.verticalCenter
                width: 20; height: 20; radius: 20;

                visible: currentDevice.hasTemperature3
                color: temp3Data.color
                Text {
                    anchors.centerIn: parent
                    text: "3"
                    color: "white"
                    font.pixelSize: 15
                    font.bold: true
                }
            }

            Rectangle { // #4
                anchors.verticalCenter: parent.verticalCenter
                width: 20; height: 20; radius: 20;

                visible: currentDevice.hasTemperature4
                color: temp4Data.color
                Text {
                    anchors.centerIn: parent
                    text: "4"
                    color: "white"
                    font.pixelSize: 15
                    font.bold: true
                }
            }

            Rectangle { // #5
                anchors.verticalCenter: parent.verticalCenter
                width: 20; height: 20; radius: 20;

                visible: currentDevice.hasTemperature5
                color: temp5Data.color
                Text {
                    anchors.centerIn: parent
                    text: "5"
                    color: "white"
                    font.pixelSize: 15
                    font.bold: true
                }
            }

            Rectangle { // #6
                anchors.verticalCenter: parent.verticalCenter
                width: 20; height: 20; radius: 20;

                visible: currentDevice.hasTemperature6
                color: temp6Data.color
                Text {
                    anchors.centerIn: parent
                    text: "6"
                    color: "white"
                    font.pixelSize: 15
                    font.bold: true
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    ChartView {
        id: rtGraph
        anchors.fill: parent
        anchors.topMargin: -28
        anchors.leftMargin: -24
        anchors.rightMargin: -24
        anchors.bottomMargin: -24

        antialiasing: true
        legend.visible: false
        backgroundRoundness: 0
        backgroundColor: "transparent"
        animationOptions: ChartView.NoAnimation

        ValueAxis { id: axisTemp; visible: false; gridVisible: false; }

        DateTimeAxis { id: axisTime; visible: true;
                       labelsFont.pixelSize: Theme.fontSizeContentSmall-1; labelsColor: legendColor;
                       color: legendColor;
                       gridLineColor: Theme.colorSeparator; }

        LineSeries {
            id: temp1Data
            useOpenGL: useOpenGL
            pointsVisible: showGraphDots
            color: Theme.colorMaterialBlue; width: 2;
            axisY: axisTemp; axisX: axisTime;
        }
        LineSeries {
            id: temp2Data
            useOpenGL: useOpenGL
            pointsVisible: showGraphDots
            color: Theme.colorMaterialDeepPurple; width: 2;
            axisY: axisTemp; axisX: axisTime;
        }
        LineSeries {
            id: temp3Data
            useOpenGL: useOpenGL
            pointsVisible: showGraphDots
            color: Theme.colorMaterialLightGreen; width: 2;
            axisY: axisTemp; axisX: axisTime;
        }
        LineSeries {
            id: temp4Data
            useOpenGL: useOpenGL
            pointsVisible: showGraphDots
            color: Theme.colorYellow; width: 2;
            axisY: axisTemp; axisX: axisTime;
        }
        LineSeries {
            id: temp5Data
            useOpenGL: useOpenGL
            pointsVisible: showGraphDots
            color: Theme.colorOrange; width: 2;
            axisY: axisTemp; axisX: axisTime;
        }
        LineSeries {
            id: temp6Data
            useOpenGL: useOpenGL
            pointsVisible: showGraphDots
            color: Theme.colorRed; width: 2;
            axisY: axisTemp; axisX: axisTime;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
