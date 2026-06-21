import QtQuick
import QtQuick.Controls

Rectangle {
    color: "#112332"
    radius: 18
    border.color: "#1f475f"
    implicitHeight: 260

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        Label { text: "Control"; color: "#dbefff"; font.pixelSize: 22 }

        Label { text: "Surge"; color: "#c0d7e7" }
        Slider {
            from: -1; to: 1; value: controlViewModel.surge
            onValueChanged: controlViewModel.surge = value
        }

        Label { text: "Yaw"; color: "#c0d7e7" }
        Slider {
            from: -1; to: 1; value: controlViewModel.yaw
            onValueChanged: controlViewModel.yaw = value
        }

        Label { text: "Ballast"; color: "#c0d7e7" }
        Slider {
            from: 0; to: 1; value: controlViewModel.ballast
            onValueChanged: controlViewModel.ballast = value
        }

        Label { text: "Lamp"; color: "#c0d7e7" }
        Slider {
            from: 0; to: 1; value: controlViewModel.lampIntensity
            onValueChanged: controlViewModel.lampIntensity = value
        }

        Switch {
            text: "Hold Position"
            checked: controlViewModel.holdPosition
            onToggled: controlViewModel.holdPosition = checked
        }
    }
}
