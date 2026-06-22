import QtQuick
import QtQuick.Controls
import "qrc:/PiSubmarine/Operator/Station/View/Theme.js" as Theme

Rectangle {
    color: timeTelemetryViewModel.backgroundColor
    radius: Theme.panelRadius
    border.color: timeTelemetryViewModel.backgroundColor === Theme.panelBackgroundFault
        ? Theme.panelBorderFault
        : Theme.panelBorder
    border.width: 1
    implicitWidth: contentRow.implicitWidth + 24
    implicitHeight: contentRow.implicitHeight + 14
    opacity: 0.96

    Row {
        id: contentRow

        anchors.centerIn: parent
        spacing: 10

        Label { text: "T"; color: Theme.textColorH3; font.pixelSize: Theme.textFontSizeH3; font.bold: true }

        Rectangle {
            width: 1
            height: 18
            anchors.verticalCenter: parent.verticalCenter
            color: Theme.textColorH4
            opacity: 0.85
        }

        Label {
            text: timeTelemetryViewModel.displayText
            color: Theme.textColorH3
            font.pixelSize: Theme.textFontSizeH3
            font.bold: true
        }
    }
}
