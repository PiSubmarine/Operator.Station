import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/PiSubmarine/Operator/Station/View/Theme.js" as Theme

Rectangle {
    color: Theme.panelBackground
    radius: Theme.panelRadius
    border.color: Theme.panelBorder
    border.width: 1
    implicitWidth: 168
    implicitHeight: 96

    GridLayout {
        anchors.fill: parent
        anchors.margins: 8
        columns: 4
        rowSpacing: 0
        columnSpacing: 0

        Label {
            text: "PCK"
            color: Theme.textColorH4
            font.pixelSize: Theme.textFontSizeH4
            font.bold: true
        }

        Label {
            text: batteryTelemetryViewModel.packVoltage.toFixed(1) + "V"
            color: Theme.textColorH4
            font.pixelSize: Theme.textFontSizeH4
        }

        Label {
            text: batteryTelemetryViewModel.packCurrent.toFixed(1) + "A"
            color: Theme.textColorH4
            font.pixelSize: Theme.textFontSizeH4
        }

        Label {
            text: batteryTelemetryViewModel.packTemperature.toFixed(1) + "C"
            color: Theme.textColorH4
            font.pixelSize: Theme.textFontSizeH4
        }

        Label {
            text: "CHG"
            color: Theme.textColorH4
            font.pixelSize: Theme.textFontSizeH4
            font.bold: true
        }

        Label {
            text: batteryTelemetryViewModel.chargerVoltage.toFixed(1) + "V"
            color: Theme.textColorH4
            font.pixelSize: Theme.textFontSizeH4
        }

        Label {
            text: batteryTelemetryViewModel.chargerCurrent.toFixed(1) + "A"
            color: Theme.textColorH4
            font.pixelSize: Theme.textFontSizeH4
        }

        Label {
            text: batteryTelemetryViewModel.chargerTemperature.toFixed(1) + "C"
            color: Theme.textColorH4
            font.pixelSize: Theme.textFontSizeH4
        }

        Label {
            text: "SOC"
            color: Theme.textColorH4
            font.pixelSize: Theme.textFontSizeH4
            font.bold: true
        }

        Label {
            text: Math.round(batteryTelemetryViewModel.stateOfCharge * 100) + "%"
            color: Theme.textColorH4
            font.pixelSize: Theme.textFontSizeH4
        }

        Label {
            text: batteryTelemetryViewModel.timeToFullText
            color: Theme.textColorH4
            font.pixelSize: Theme.textFontSizeH4
        }

        Label {
            text: batteryTelemetryViewModel.timeToEmptyText
            color: Theme.textColorH4
            font.pixelSize: Theme.textFontSizeH4
        }
    }
}
