import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

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

        Component.onCompleted: displayedColor = targetColor
        onTargetColorChanged: updateDisplayedColor()

        ColorAnimation {
            id: fadeToNeutral

            target: indicator
            property: "displayedColor"
            duration: indicator.lingerDuration
            easing.type: Easing.OutCubic
        }
    }

    implicitWidth: 80
    implicitHeight: 100
    radius: 12
    color: "#f0091823"
    border.color: "#2d617a"
    border.width: 1
    clip: true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 6

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "LMP"
                color: "#eef7ff"
                font.pixelSize: 12
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                text: Math.round(lampTelemetryViewModel.intensity * 100) + "%"
                color: "#dbefff"
                font.pixelSize: 12
                font.bold: true
            }
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
                targetColor: lampTelemetryViewModel.hasOvercurrentFault ? "#ff5f56" : neutralColor
            }
            IndicatorLabel {
                text: "OT"
                targetColor: lampTelemetryViewModel.hasOvertemperatureFault
                    ? "#ff5f56"
                    : lampTelemetryViewModel.hasTemperatureWarning
                        ? "#ffd166"
                        : neutralColor
            }
            IndicatorLabel {
                text: "UV"
                targetColor: lampTelemetryViewModel.hasUndervoltageFault ? "#ff5f56" : neutralColor
            }
            IndicatorLabel {
                text: "OV"
                targetColor: lampTelemetryViewModel.hasOvervoltageFault ? "#ff5f56" : neutralColor
            }
            IndicatorLabel {
                text: "OL"
                targetColor: lampTelemetryViewModel.hasOpenLoadFault ? "#ff5f56" : neutralColor
            }
        }
    }
}
