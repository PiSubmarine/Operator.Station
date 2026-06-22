import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#f0091823"
    radius: 16
    border.color: "#2d617a"
    border.width: 1
    implicitWidth: 160
    implicitHeight: 96

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "LMP"
                color: "#eef7ff"
                font.pixelSize: 14
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                text: Math.round(controlViewModel.lampIntensity * 100) + "%"
                color: "#dbefff"
                font.pixelSize: 14
                font.bold: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 32
                radius: 10
                border.width: 1
                border.color: lampIncreaseTapHandler.pressed ? "#8fd3ff" : "#2d617a"
                color: lampIncreaseTapHandler.pressed ? "#2f6f8d" : (lampIncreaseHoverHandler.hovered ? "#244e65" : "#173447")

                Label {
                    anchors.centerIn: parent
                    text: "INC"
                    color: "#eef7ff"
                    font.pixelSize: 12
                    font.bold: true
                }

                HoverHandler {
                    id: lampIncreaseHoverHandler
                }

                TapHandler {
                    id: lampIncreaseTapHandler
                    onTapped: controlViewModel.IncreaseLampIntensity()
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 32
                radius: 10
                border.width: 1
                border.color: lampDecreaseTapHandler.pressed ? "#8fd3ff" : "#2d617a"
                color: lampDecreaseTapHandler.pressed ? "#2f6f8d" : (lampDecreaseHoverHandler.hovered ? "#244e65" : "#173447")

                Label {
                    anchors.centerIn: parent
                    text: "DEC"
                    color: "#eef7ff"
                    font.pixelSize: 12
                    font.bold: true
                }

                HoverHandler {
                    id: lampDecreaseHoverHandler
                }

                TapHandler {
                    id: lampDecreaseTapHandler
                    onTapped: controlViewModel.DecreaseLampIntensity()
                }
            }
        }
    }
}
