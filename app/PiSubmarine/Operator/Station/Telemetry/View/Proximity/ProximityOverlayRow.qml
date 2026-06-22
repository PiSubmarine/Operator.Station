import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/PiSubmarine/Operator/Station/View/Theme.js" as Theme

GridLayout {
    columns: 2
    rowSpacing: 0
    columnSpacing: 12

    Label {
        text: "PRX"
        color: Theme.textColorH4
        font.pixelSize: Theme.textFontSizeH4
        font.bold: true
    }

    Label {
        text: proximityTelemetryViewModel.hasDistance
            ? proximityTelemetryViewModel.distanceMeters.toFixed(1) + " m"
            : "--.- m"
        color: Theme.textColorH4
        font.pixelSize: Theme.textFontSizeH4
    }
}
