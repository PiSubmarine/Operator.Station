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

        Label { text: "Proximity"; color: "#dbefff"; font.pixelSize: 22 }
        Label {
            text: proximityTelemetryViewModel.hasDistance
                ? "Distance: " + proximityTelemetryViewModel.distanceMeters.toFixed(2) + " m"
                : "Distance unavailable"
            color: "#c0d7e7"
        }
    }
}
