import QtQuick
import QtQuick.Controls
import "qrc:/PiSubmarine/Operator/Station/View/Theme.js" as Theme

Rectangle {
    readonly property bool isFault: controlStatusViewModel.statusKind === "fault"
    readonly property bool isWarning: controlStatusViewModel.statusKind === "warning"

    color: isFault
        ? Theme.panelBackgroundFault
        : (isWarning ? Theme.panelBackgroundWarning : Theme.panelBackground)
    radius: Theme.panelRadius
    border.color: isFault
        ? Theme.panelBorderFault
        : (isWarning
            ? Theme.panelBorderWarning
            : Theme.panelBorder)
    border.width: 1
    implicitWidth: contentRow.implicitWidth + 24
    implicitHeight: contentRow.implicitHeight + 14
    opacity: 0.96

    Row {
        id: contentRow

        anchors.centerIn: parent
        spacing: 10

        Label {
            text: controlStatusViewModel.symbol
            color: Theme.textColorH3
            font.pixelSize: Theme.textFontSizeH3
            font.bold: true
        }
    }

    TapHandler {
        onTapped: controlStatusViewModel.ToggleBindingDialog()
    }
}
