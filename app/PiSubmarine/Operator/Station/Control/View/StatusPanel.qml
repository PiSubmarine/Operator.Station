import QtQuick
import QtQuick.Controls

Rectangle {
    color: controlStatusViewModel.backgroundColor
    radius: 14
    border.color: "#dbefff"
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
            color: "#f7fbff"
            font.pixelSize: 18
            font.bold: true
        }
    }

    TapHandler {
        onTapped: controlStatusViewModel.ToggleBindingDialog()
    }
}
