import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GridLayout {
    columns: 2
    rowSpacing: 0
    columnSpacing: 12

    Label {
        text: "BAL"
        color: "#9db7c7"
        font.pixelSize: 14
        font.bold: true
    }

    Label {
        text: ballastTelemetryViewModel.hasPosition
            ? Math.round(ballastTelemetryViewModel.position * 100) + " %"
            : "--- %"
        color: "#dbefff"
        font.pixelSize: 12
    }
}
