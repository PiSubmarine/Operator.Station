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

        Label { text: "Input"; color: "#dbefff"; font.pixelSize: 22 }

        Label { text: "Surge"; color: "#c0d7e7" }
        Slider {
            from: -1; to: 1; value: inputViewModel.surge
            onValueChanged: inputViewModel.surge = value
        }

        Label { text: "Yaw"; color: "#c0d7e7" }
        Slider {
            from: -1; to: 1; value: inputViewModel.yaw
            onValueChanged: inputViewModel.yaw = value
        }

        Label { text: "Ballast"; color: "#c0d7e7" }
        Slider {
            from: 0; to: 1; value: inputViewModel.ballast
            onValueChanged: inputViewModel.ballast = value
        }

        Label { text: "Lamp"; color: "#c0d7e7" }
        Slider {
            from: 0; to: 1; value: inputViewModel.lampIntensity
            onValueChanged: inputViewModel.lampIntensity = value
        }

        Switch {
            text: "Hold Position"
            checked: inputViewModel.holdPosition
            onToggled: inputViewModel.holdPosition = checked
        }
    }
}
