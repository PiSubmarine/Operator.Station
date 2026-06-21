import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property var overlayViewModels: []

    z: 1

    Column {
        id: overlayColumn

        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(root.width - 48, 420)
        spacing: 12

        Repeater {
            model: root.overlayViewModels

            delegate: Rectangle {
                required property var modelData

                visible: modelData !== null && modelData.isOverlayVisible
                width: overlayColumn.width
                height: visible ? overlayLabel.implicitHeight + 28 : 0
                radius: 16
                color: modelData !== null ? modelData.overlayBackgroundColor : "transparent"
                border.width: visible ? 1 : 0
                border.color: "#dbefff"
                opacity: 0.94

                Label {
                    id: overlayLabel

                    anchors.centerIn: parent
                    text: modelData !== null ? modelData.overlayMessage : ""
                    color: "#f7fbff"
                    font.pixelSize: 24
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
