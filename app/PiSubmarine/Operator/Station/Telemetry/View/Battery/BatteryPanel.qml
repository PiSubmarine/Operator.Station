import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#f0091823"
    radius: 16
    border.color: "#2d617a"
    border.width: 1
    implicitWidth: 160
    implicitHeight: 96

    GridLayout {
        anchors.fill: parent
        anchors.margins: 8
        columns: 4
        rowSpacing: 0
        columnSpacing: 0

        Label {
            text: "PCK"
            color: "#9db7c7"
            font.pixelSize: 14
            font.bold: true
        }

        Label {
            text: batteryTelemetryViewModel.packVoltage.toFixed(1) + "V"
            color: "#dbefff"
            font.pixelSize: 12
        }

        Label {
            text: batteryTelemetryViewModel.packCurrent.toFixed(1) + "A"
            color: "#dbefff"
            font.pixelSize: 12
        }

        Label {
            text: batteryTelemetryViewModel.packTemperature.toFixed(1) + "C"
            color: "#dbefff"
            font.pixelSize: 12
        }

        Label {
            text: "CHG"
            color: "#9db7c7"
            font.pixelSize: 14
            font.bold: true
        }

        Label {
            text: batteryTelemetryViewModel.chargerVoltage.toFixed(1) + "V"
            color: "#dbefff"
            font.pixelSize: 12
        }

        Label {
            text: batteryTelemetryViewModel.chargerCurrent.toFixed(1) + "A"
            color: "#dbefff"
            font.pixelSize: 12
        }

        Label {
            text: batteryTelemetryViewModel.chargerTemperature.toFixed(1) + "C"
            color: "#dbefff"
            font.pixelSize: 12
        }

        Label {
            text: "SOC"
            color: "#9db7c7"
            font.pixelSize: 14
            font.bold: true
        }

        Label {
            text: Math.round(batteryTelemetryViewModel.stateOfCharge * 100) + "%"
            color: "#f7ffbf"
            font.pixelSize: 12
        }

        Label {
            text: batteryTelemetryViewModel.timeToFullText
            color: "#c0d7e7"
            font.pixelSize: 12
        }

        Label {
            text: batteryTelemetryViewModel.timeToEmptyText
            color: "#c0d7e7"
            font.pixelSize: 12
        }
    }
}
