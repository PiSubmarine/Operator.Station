import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GridLayout {
    columns: 2
    rowSpacing: 0
    columnSpacing: 12

    Label {
        text: "PRX"
        color: "#9db7c7"
        font.pixelSize: 14
        font.bold: true
    }

    Label {
        text: proximityTelemetryViewModel.hasDistance
            ? proximityTelemetryViewModel.distanceMeters.toFixed(1) + " m"
            : "--.- m"
        color: "#dbefff"
        font.pixelSize: 12
    }
}
