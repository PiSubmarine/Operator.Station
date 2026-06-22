import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/PiSubmarine/Operator/Station/View/Theme.js" as Theme

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
                border.color: modelData !== null && modelData.overlayBackgroundColor === Theme.panelBackgroundFault
                    ? Theme.panelBorderFault
                    : Theme.panelBorder
                opacity: 0.94

                Label {
                    id: overlayLabel

                    anchors.centerIn: parent
                    text: modelData !== null ? modelData.overlayMessage : ""
                    color: Theme.textColorH1
                    font.pixelSize: Theme.textFontSizeH1
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
