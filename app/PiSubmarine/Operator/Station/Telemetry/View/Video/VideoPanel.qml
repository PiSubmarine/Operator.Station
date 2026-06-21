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

        Label { text: "Video Telemetry"; color: "#dbefff"; font.pixelSize: 22 }
        Label { text: videoTelemetryViewModel.operationalState; color: "#c0d7e7" }
        Label { text: videoTelemetryViewModel.isStreamingEnabled ? "Streaming enabled" : "Streaming disabled"; color: "#c0d7e7" }
        Label { text: "Subscribers: " + videoTelemetryViewModel.subscribers; color: "#c0d7e7" }
        Label {
            text: videoTelemetryViewModel.hasFault ? "Fault detected" : "No faults"
            color: videoTelemetryViewModel.hasFault ? "#ff8f70" : "#c0d7e7"
        }
    }
}
