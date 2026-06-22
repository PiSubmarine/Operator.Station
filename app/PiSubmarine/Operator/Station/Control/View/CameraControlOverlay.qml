import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#f0091823"
    radius: 16
    border.color: "#2d617a"
    border.width: 1
    implicitWidth: 220
    implicitHeight: 180

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

        HoverHandler {
            id: hoverHandler
        }

        TapHandler {
            id: tapHandler
            onTapped: parent.pressed()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        GridLayout {
            Layout.fillWidth: true
            columns: 3
            columnSpacing: 8
            rowSpacing: 8

            Label {
                text: "CAM"
                color: "#eef7ff"
                font.pixelSize: 14
                font.bold: true
            }

            StateButton {
                buttonText: "ENA"
                active: controlViewModel.cameraEnabled
                onPressed: controlViewModel.EnableCamera()
            }

            StateButton {
                buttonText: "DIS"
                active: !controlViewModel.cameraEnabled
                onPressed: controlViewModel.DisableCamera()
            }

            Label {
                text: "FCS"
                color: "#eef7ff"
                font.pixelSize: 14
                font.bold: true
            }

            StateButton {
                buttonText: "AUT"
                active: controlViewModel.autoFocusEnabled
                onPressed: controlViewModel.SetAutoFocusEnabled()
            }

            StateButton {
                buttonText: "MAN"
                active: !controlViewModel.autoFocusEnabled
                onPressed: controlViewModel.SetManualFocusEnabled()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: "NEA"
                color: "#c0d7e7"
                font.pixelSize: 12
                font.bold: true
            }

            Slider {
                Layout.fillWidth: true
                from: 0
                to: 1
                enabled: !controlViewModel.autoFocusEnabled
                value: controlViewModel.manualFocusPosition
                onMoved: controlViewModel.SetManualFocusPosition(value)
            }

            Label {
                text: "FAR"
                color: "#c0d7e7"
                font.pixelSize: 12
                font.bold: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "QUA"
                color: "#eef7ff"
                font.pixelSize: 14
                font.bold: true
            }

            StateButton {
                buttonText: "LQ"
                active: controlViewModel.streamProfile === 0
                onPressed: controlViewModel.SetLowQualityStreamProfile()
            }

            StateButton {
                buttonText: "MD"
                active: controlViewModel.streamProfile === 1
                onPressed: controlViewModel.SetMediumQualityStreamProfile()
            }

            StateButton {
                buttonText: "HQ"
                active: controlViewModel.streamProfile === 2
                onPressed: controlViewModel.SetHighQualityStreamProfile()
            }
        }
    }
}
