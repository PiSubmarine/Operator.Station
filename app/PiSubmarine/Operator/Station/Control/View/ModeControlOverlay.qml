import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#f0091823"
    radius: 16
    border.color: "#2d617a"
    border.width: 1
    implicitWidth: 220
    implicitHeight: 56

    component StateButton: Rectangle {
        property string buttonText: ""
        property bool active: false
        signal pressed()

        Layout.fillWidth: true
        Layout.preferredHeight: 28
        radius: 10
        border.width: 1
        border.color: tapHandler.pressed ? "#b9ffd0" : (active ? "#73d496" : "#2d617a")
        color: tapHandler.pressed ? "#2a7e50" : (active ? "#1d6a42" : (hoverHandler.hovered ? "#244e65" : "#173447"))

        Label {
            anchors.centerIn: parent
            text: parent.buttonText
            color: "#eef7ff"
            font.pixelSize: 12
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
            color: "#eef7ff"
            font.pixelSize: 14
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
