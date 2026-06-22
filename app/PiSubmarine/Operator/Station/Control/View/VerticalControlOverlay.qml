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
    implicitHeight: controlColumn.implicitHeight + 24

    component StateButton: Rectangle {
        property string buttonText: ""
        property bool active: false
        signal pressed()

        Layout.fillWidth: true
        Layout.preferredHeight: 28
        radius: Theme.buttonRadius
        border.width: 1
        border.color: tapHandler.pressed ? Theme.buttonPressedStrongBorder : (active ? Theme.buttonActiveBorder : Theme.panelBorder)
        color: tapHandler.pressed ? Theme.buttonPressedStrongBackground : (active ? Theme.buttonActiveBackground : (hoverHandler.hovered ? Theme.buttonHoverBackground : Theme.panelBackground))

        Label {
            anchors.centerIn: parent
            text: parent.buttonText
            color: Theme.textColorH4
            font.pixelSize: Theme.textFontSizeH4
            font.bold: true
        }

        HoverHandler { id: hoverHandler }
        TapHandler {
            id: tapHandler
            onTapped: parent.pressed()
        }
    }

    ColumnLayout {
        id: controlColumn
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: "DEP"
                color: Theme.textColorH3
                font.pixelSize: Theme.textFontSizeH3
                font.bold: true
            }

            StateButton {
                buttonText: "HLD"
                active: controlViewModel.verticalMode === 0
                onPressed: controlViewModel.SetVerticalKeepCurrentMode()
            }

            StateButton {
                buttonText: "BST"
                active: controlViewModel.verticalMode === 1
                onPressed: controlViewModel.SetVerticalBallastPositionMode()
            }

            StateButton {
                buttonText: "TGT"
                active: controlViewModel.verticalMode === 2
                onPressed: controlViewModel.SetVerticalDepthTargetMode()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            visible: controlViewModel.verticalMode === 1

            Label {
                text: "BST"
                color: Theme.textColorH4
                font.pixelSize: Theme.textFontSizeH4
                font.bold: true
            }

            Label {
                text: Math.round(controlViewModel.ballastPosition * 100) + "%"
                color: Theme.textColorH4
                font.pixelSize: Theme.textFontSizeH4
                font.bold: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            visible: controlViewModel.verticalMode === 2

            Label {
                text: "TGT"
                color: Theme.textColorH4
                font.pixelSize: Theme.textFontSizeH4
                font.bold: true
            }

            Label {
                text: controlViewModel.depthTargetMeters.toFixed(1) + " m"
                color: Theme.textColorH4
                font.pixelSize: Theme.textFontSizeH4
                font.bold: true
            }
        }
    }
}
