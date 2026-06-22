import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GridLayout {
    columns: 2
    rowSpacing: 0
    columnSpacing: 12

    Label {
        text: "DEP"
        color: "#9db7c7"
        font.pixelSize: 14
        font.bold: true
    }

    Label {
        text: depthTelemetryViewModel.hasDepth
            ? depthTelemetryViewModel.depthMeters.toFixed(2) + " m"
            : "--.-- m"
        color: "#dbefff"
        font.pixelSize: 12
    }
}
