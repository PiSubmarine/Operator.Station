import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PiSubmarine.Operator.Station 1.0

ApplicationWindow {
    visible: true
    width: 1440
    height: 860
    color: "#08111a"
    title: "PiSubmarine Operator Station"

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "#03070b"
            radius: 20
            border.color: "#163348"

            VideoSurfaceItem {
                id: videoSurface
                objectName: "videoSurface"
                anchors.fill: parent
            }
        }

        ColumnLayout {
            Layout.preferredWidth: 380
            Layout.fillHeight: true
            spacing: 16

            Loader {
                Layout.fillWidth: true
                source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Battery/BatteryPanel.qml"
            }

            Loader {
                Layout.fillWidth: true
                source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Motor/MotorPanel.qml"
            }

            Loader {
                Layout.fillWidth: true
                source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Lamp/LampPanel.qml"
            }

            Loader {
                Layout.fillWidth: true
                source: "qrc:/PiSubmarine/Operator/Station/Input/View/ControlPanel.qml"
            }
        }
    }
}
