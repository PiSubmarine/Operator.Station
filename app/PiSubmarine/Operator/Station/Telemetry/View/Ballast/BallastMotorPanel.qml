import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/PiSubmarine/Operator/Station/View/Theme.js" as Theme

Rectangle {
    id: root

    property QtObject viewModel: null
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

    implicitWidth: 168
    implicitHeight: 64
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
                text: viewModel !== null ? viewModel.panelLabel : "Motor"
                color: Theme.textColorH4
                font.pixelSize: Theme.textFontSizeH4
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                text: viewModel !== null ? Math.round(viewModel.driveEffortPercent) + "%" : "0%"
                color: Theme.textColorH4
                font.pixelSize: Theme.textFontSizeH4
                font.bold: true
            }
        }

        Item {
            Layout.fillHeight: true
        }

        GridLayout {
            columns: 5
            columnSpacing: 6
            rowSpacing: 4

            IndicatorLabel {
                text: "OC"
                targetColor: viewModel !== null && viewModel.hasOvercurrentFault ? Theme.textFault : neutralColor
            }
            IndicatorLabel {
                text: "OT"
                targetColor: viewModel !== null && viewModel.hasOvertemperatureFault
                    ? Theme.textFault
                    : viewModel !== null && viewModel.hasTemperatureWarning
                        ? Theme.textWarning
                        : neutralColor
            }
            IndicatorLabel {
                text: "UV"
                targetColor: viewModel !== null && viewModel.hasUndervoltageFault ? Theme.textFault : neutralColor
            }
            IndicatorLabel {
                text: "OV"
                targetColor: viewModel !== null && viewModel.hasOvervoltageFault ? Theme.textFault : neutralColor
            }
            IndicatorLabel {
                text: "OL"
                targetColor: viewModel !== null && viewModel.hasOpenLoadFault ? Theme.textFault : neutralColor
            }
        }
    }
}
