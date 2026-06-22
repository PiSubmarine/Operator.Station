import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/PiSubmarine/Operator/Station/View/Theme.js" as Theme

Rectangle {
    id: root

    property int lingerDuration: 1200

    component IndicatorLabel: Label {
        id: indicator

        property color neutralColor: Theme.textColorH4
        property color targetColor: neutralColor
        property color displayedColor: neutralColor
        property int lingerDuration: root.lingerDuration

        color: displayedColor
        font.bold: true
        font.pixelSize: Theme.textFontSizeH4

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
    radius: Theme.panelRadius
    color: Theme.panelBackground
    border.color: Theme.panelBorder
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
                color: Theme.textColorH4
                font.pixelSize: Theme.textFontSizeH4
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                text: Math.round(lampTelemetryViewModel.intensity * 100) + "%"
                color: Theme.textColorH4
                font.pixelSize: Theme.textFontSizeH4
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
                targetColor: lampTelemetryViewModel.hasOvercurrentFault ? Theme.textFault : neutralColor
            }
            IndicatorLabel {
                text: "OT"
                targetColor: lampTelemetryViewModel.hasOvertemperatureFault
                    ? Theme.textFault
                    : lampTelemetryViewModel.hasTemperatureWarning
                        ? Theme.textWarning
                        : neutralColor
            }
            IndicatorLabel {
                text: "UV"
                targetColor: lampTelemetryViewModel.hasUndervoltageFault ? Theme.textFault : neutralColor
            }
            IndicatorLabel {
                text: "OV"
                targetColor: lampTelemetryViewModel.hasOvervoltageFault ? Theme.textFault : neutralColor
            }
            IndicatorLabel {
                text: "OL"
                targetColor: lampTelemetryViewModel.hasOpenLoadFault ? Theme.textFault : neutralColor
            }
        }
    }
}
