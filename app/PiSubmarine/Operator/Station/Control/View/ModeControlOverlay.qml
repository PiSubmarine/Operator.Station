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
    implicitHeight: 56

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

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Label {
            text: "PLT"
            color: Theme.textColorH3
            font.pixelSize: Theme.textFontSizeH3
            font.bold: true
        }

        StateButton {
            buttonText: "HLD"
            active: controlViewModel.holdPositionMode
            onPressed: controlViewModel.SetHoldPositionMode()
        }

        StateButton {
            buttonText: "MAN"
            active: !controlViewModel.holdPositionMode
            onPressed: controlViewModel.SetManualMode()
        }
    }
}
