import QtQuick
import QtQuick.Controls

Rectangle {
    color: "#112332"
    radius: 18
    border.color: "#1f475f"
    implicitHeight: 140

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 8

        Label { text: "Battery"; color: "#dbefff"; font.pixelSize: 22 }
        Label { text: "Pack: " + batteryTelemetryViewModel.packVoltage.toFixed(2) + " V"; color: "#c0d7e7" }
        Label { text: "Current: " + batteryTelemetryViewModel.packCurrent.toFixed(2) + " A"; color: "#c0d7e7" }
        Label { text: "Charge: " + Math.round(batteryTelemetryViewModel.stateOfCharge * 100) + " %"; color: "#c0d7e7" }
        Label { text: "Temp: " + batteryTelemetryViewModel.packTemperature.toFixed(1) + " C"; color: "#c0d7e7" }
    }
}
