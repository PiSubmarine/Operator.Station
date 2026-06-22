import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property color neutralPrimaryColor: "#3b82b6"
    property color targetPrimaryColor: lampTelemetryViewModel.hasOvercurrentFault
        || lampTelemetryViewModel.hasOvertemperatureFault
        || lampTelemetryViewModel.hasUndervoltageFault
        || lampTelemetryViewModel.hasOvervoltageFault
        || lampTelemetryViewModel.hasOpenLoadFault
        ? "#ff5f56"
        : lampTelemetryViewModel.hasTemperatureWarning
            ? "#ffd166"
            : neutralPrimaryColor
    property color displayedPrimaryColor: targetPrimaryColor
    property int lingerDuration: 1200

    function updateDisplayedPrimaryColor() {
        if (targetPrimaryColor === neutralPrimaryColor) {
            fadePrimaryColor.stop()
            fadePrimaryColor.from = displayedPrimaryColor
            fadePrimaryColor.to = neutralPrimaryColor
            fadePrimaryColor.start()
            return
        }

        fadePrimaryColor.stop()
        displayedPrimaryColor = targetPrimaryColor
    }

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

    function alphaColor(colorValue, alphaValue) {
        return Qt.rgba(colorValue.r, colorValue.g, colorValue.b, alphaValue)
    }

    implicitWidth: 76
    implicitHeight: 110
    radius: 12
    color: alphaColor(displayedPrimaryColor, 0.35)
    border.color: alphaColor(displayedPrimaryColor, 0.85)
    border.width: 1
    clip: true

    Component.onCompleted: displayedPrimaryColor = targetPrimaryColor
    onTargetPrimaryColorChanged: updateDisplayedPrimaryColor()

    ColorAnimation {
        id: fadePrimaryColor

        target: root
        property: "displayedPrimaryColor"
        duration: root.lingerDuration
        easing.type: Easing.OutCubic
    }

    Rectangle {
        width: parent.width
        height: parent.height * Math.max(0.0, Math.min(1.0, lampTelemetryViewModel.intensity))
        radius: root.radius
        color: displayedPrimaryColor
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 6

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "LMP"
                color: "#eef7ff"
                font.pixelSize: 14
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }

        }

        Item {
            Layout.fillHeight: true
        }

        Rectangle {
            Layout.fillWidth: true
            color: "#b0050b10"
            radius: 8
            border.width: 1
            border.color: "#33000000"

            implicitHeight: indicatorGrid.implicitHeight + 10

            GridLayout {
                id: indicatorGrid

                anchors.fill: parent
                anchors.margins: 5
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
}
