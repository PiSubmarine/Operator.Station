import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/PiSubmarine/Operator/Station/View/Theme.js" as Theme

GridLayout {
    columns: 2
    rowSpacing: 0
    columnSpacing: 12

    Label {
        text: "DEP"
        color: Theme.textColorH4
        font.pixelSize: Theme.textFontSizeH4
        font.bold: true
    }

    Label {
        text: depthTelemetryViewModel.hasDepth
            ? depthTelemetryViewModel.depthMeters.toFixed(2) + " m"
            : "--.-- m"
        color: Theme.textColorH4
        font.pixelSize: Theme.textFontSizeH4
    }
}
