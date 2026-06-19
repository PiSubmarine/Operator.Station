import QtQuick
import QtQuick.Controls

Rectangle {
    color: "#112332"
    radius: 18
    border.color: "#1f475f"
    implicitHeight: 108

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 8

        Label { text: "Ballast"; color: "#dbefff"; font.pixelSize: 22 }
        Label {
            text: ballastTelemetryViewModel.hasPosition
                ? "Position: " + Math.round(ballastTelemetryViewModel.position * 100) + " %"
                : "Position unavailable"
            color: "#c0d7e7"
        }
    }
}
