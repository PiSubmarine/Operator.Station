import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/PiSubmarine/Operator/Station/View/Theme.js" as Theme

Rectangle {
    color: Theme.panelBackground
    radius: Theme.panelRadius
    border.color: Theme.panelBorder
    border.width: 1
    implicitWidth: 220
    implicitHeight: 96

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "LMP"
                color: Theme.textColorH3
                font.pixelSize: Theme.textFontSizeH3
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                text: Math.round(controlViewModel.lampIntensity * 100) + "%"
                color: Theme.textColorH3
                font.pixelSize: Theme.textFontSizeH3
                font.bold: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 32
                radius: Theme.buttonRadius
                border.width: 1
                border.color: lampIncreaseTapHandler.pressed ? Theme.buttonPressedBorder : Theme.panelBorder
                color: lampIncreaseTapHandler.pressed ? Theme.buttonPressedBackground : (lampIncreaseHoverHandler.hovered ? Theme.buttonHoverBackground : Theme.panelBackground)

                Label {
                    anchors.centerIn: parent
                    text: "INC"
                    color: Theme.textColorH4
                    font.pixelSize: Theme.textFontSizeH4
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
                radius: Theme.buttonRadius
                border.width: 1
                border.color: lampDecreaseTapHandler.pressed ? Theme.buttonPressedBorder : Theme.panelBorder
                color: lampDecreaseTapHandler.pressed ? Theme.buttonPressedBackground : (lampDecreaseHoverHandler.hovered ? Theme.buttonHoverBackground : Theme.panelBackground)

                Label {
                    anchors.centerIn: parent
                    text: "DEC"
                    color: Theme.textColorH4
                    font.pixelSize: Theme.textFontSizeH4
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
