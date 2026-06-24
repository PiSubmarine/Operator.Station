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
    implicitHeight: cameraColumn.implicitHeight + 24

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

        HoverHandler {
            id: hoverHandler
        }

        TapHandler {
            id: tapHandler
            onTapped: parent.pressed()
        }
    }

    ColumnLayout {
        id: cameraColumn
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
                color: Theme.textColorH3
                font.pixelSize: Theme.textFontSizeH3
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
                color: Theme.textColorH3
                font.pixelSize: Theme.textFontSizeH3
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
                color: Theme.textColorH4
                font.pixelSize: Theme.textFontSizeH4
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
                color: Theme.textColorH4
                font.pixelSize: Theme.textFontSizeH4
                font.bold: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "QUA"
                color: Theme.textColorH3
                font.pixelSize: Theme.textFontSizeH3
                font.bold: true
            }

            StateButton {
                buttonText: "LQ"
                active: controlViewModel.streamProfile === 0
                onPressed: controlViewModel.SetLowQualityStreamProfile()
            }

            StateButton {
                buttonText: "MQ"
                active: controlViewModel.streamProfile === 1
                onPressed: controlViewModel.SetMediumQualityStreamProfile()
            }

            StateButton {
                buttonText: "HQ"
                active: controlViewModel.streamProfile === 2
                onPressed: controlViewModel.SetHighQualityStreamProfile()
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "PIT"
                color: Theme.textColorH3
                font.pixelSize: Theme.textFontSizeH3
                font.bold: true
            }

            Label {
                text: controlViewModel.gimbalPitchDegrees.toFixed(1) + " deg"
                color: Theme.textColorH4
                font.pixelSize: Theme.textFontSizeH4
                font.bold: true
            }
        }
    }
}
