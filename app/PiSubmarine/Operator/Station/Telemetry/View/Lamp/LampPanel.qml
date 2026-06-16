import QtQuick
import QtQuick.Controls

Rectangle {
    color: "#112332"
    radius: 18
    border.color: "#1f475f"
    implicitHeight: 120

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 8

        Label { text: "Lamp"; color: "#dbefff"; font.pixelSize: 22 }
        Label { text: lampTelemetryViewModel.isActive ? "Active" : "Idle"; color: "#c0d7e7" }
        Label { text: lampTelemetryViewModel.hasFault ? "Fault detected" : "No faults"; color: lampTelemetryViewModel.hasFault ? "#ff8f70" : "#c0d7e7" }
        Label { text: lampTelemetryViewModel.hasWarning ? "Warning present" : "No warnings"; color: lampTelemetryViewModel.hasWarning ? "#ffd166" : "#c0d7e7" }
    }
}
