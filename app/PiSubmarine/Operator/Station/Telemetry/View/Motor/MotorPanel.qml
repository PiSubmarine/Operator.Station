import QtQuick
import QtQuick.Controls

Rectangle {
    property QtObject viewModel: null

    color: "#112332"
    radius: 18
    border.color: "#1f475f"
    implicitHeight: 120

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 8

        Label { text: "Motor"; color: "#dbefff"; font.pixelSize: 22 }
        Label { text: viewModel !== null ? viewModel.operationalState : "Unknown"; color: "#c0d7e7" }
        Label {
            text: viewModel !== null && viewModel.hasFault ? "Fault detected" : "No faults"
            color: viewModel !== null && viewModel.hasFault ? "#ff8f70" : "#c0d7e7"
        }
        Label {
            text: viewModel !== null && viewModel.hasWarning ? "Warning present" : "No warnings"
            color: viewModel !== null && viewModel.hasWarning ? "#ffd166" : "#c0d7e7"
        }
    }
}
