import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property QtObject viewModel: null
    property int lingerDuration: 1200

    component IndicatorLabel: Label {
        id: indicator

        property color neutralColor: "#d7e9f6"
        property color targetColor: neutralColor
        property color displayedColor: neutralColor
        property int lingerDuration: root.lingerDuration

        color: displayedColor
        font.bold: true
        font.pixelSize: 12

        function updateDisplayedColor() {
            if (targetColor === neutralColor) {
                fadeToNeutral.stop()
                fadeToNeutral.from = displayedColor
                fadeToNeutral.to = neutralColor
                fadeToNeutral.start()
                return
            }

            fadeToNeutral.stop()
            displayedColor = targetColor
        }

        Component.onCompleted: {
            displayedColor = targetColor
        }

        onTargetColorChanged: {
            updateDisplayedColor()
        }

        ColorAnimation {
            id: fadeToNeutral

            target: indicator
            property: "displayedColor"
            duration: indicator.lingerDuration
            easing.type: Easing.OutCubic
        }
    }

    function alphaColor(hexColor, alphaValue) {
        return Qt.rgba(
            parseInt(hexColor.slice(1, 3), 16) / 255.0,
            parseInt(hexColor.slice(3, 5), 16) / 255.0,
            parseInt(hexColor.slice(5, 7), 16) / 255.0,
            alphaValue)
    }

    implicitWidth: 64
    implicitHeight: 110
    radius: 12
    color: viewModel !== null ? alphaColor(viewModel.primaryColor, 0.35) : "#553b82b6"
    border.color: viewModel !== null ? alphaColor(viewModel.primaryColor, 0.85) : "#aa3b82b6"
    border.width: 1
    clip: true

    Rectangle {
        width: parent.width
        height: parent.height * (viewModel !== null ? Math.max(0.0, Math.min(1.0, viewModel.driveEffortPercent / 100.0)) : 0.0)
        radius: root.radius
        color: viewModel !== null ? viewModel.primaryColor : "#3b82b6"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: viewModel !== null && viewModel.fillFromTop ? parent.top : undefined
        anchors.bottom: viewModel !== null && !viewModel.fillFromTop ? parent.bottom : undefined
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 6

        Label {
            text: viewModel !== null ? viewModel.panelLabel : "Motor"
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            color: "#eef7ff"
            font.pixelSize: 14
            font.bold: true
        }

        Item {
            Layout.fillHeight: true
        }

        GridLayout {
            columns: 2
            columnSpacing: 6
            rowSpacing: 4

            IndicatorLabel {
                text: "OC"
                targetColor: viewModel !== null && viewModel.hasOvercurrentFault ? "#ff5f56" : neutralColor
            }
            IndicatorLabel {
                text: "OT"
                targetColor: viewModel !== null && viewModel.hasOvertemperatureFault
                    ? "#ff5f56"
                    : viewModel !== null && viewModel.hasTemperatureWarning
                        ? "#ffd166"
                        : neutralColor
            }
            IndicatorLabel {
                text: "UV"
                targetColor: viewModel !== null && viewModel.hasUndervoltageFault ? "#ff5f56" : neutralColor
            }
            IndicatorLabel {
                text: "OV"
                targetColor: viewModel !== null && viewModel.hasOvervoltageFault ? "#ff5f56" : neutralColor
            }
            IndicatorLabel {
                text: "OL"
                targetColor: viewModel !== null && viewModel.hasOpenLoadFault ? "#ff5f56" : neutralColor
            }
        }
    }
}
