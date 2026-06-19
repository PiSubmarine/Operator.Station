import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.freedesktop.gstreamer.Qt6D3D11VideoItem 1.0

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
            Layout.minimumWidth: 320
            Layout.preferredWidth: 1000
            color: "#03070b"
            radius: 20
            border.color: "#163348"
            clip: true

            GstD3D11Qt6VideoItem {
                id: videoSurface
                objectName: "videoSurface"
                anchors.centerIn: parent
                width: parent.width
                height: parent.height
            }
        }

        ScrollView {
            Layout.preferredWidth: 380
            Layout.minimumWidth: 380
            Layout.maximumWidth: 380
            Layout.fillHeight: true
            clip: true

            ColumnLayout {
                width: 380
                spacing: 16

                Loader {
                    Layout.fillWidth: true
                    source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Battery/BatteryPanel.qml"
                }

                Loader {
                    Layout.fillWidth: true
                    source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Depth/DepthPanel.qml"
                }

                Loader {
                    Layout.fillWidth: true
                    source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Proximity/ProximityPanel.qml"
                }

                Loader {
                    Layout.fillWidth: true
                    source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Ballast/BallastPanel.qml"
                }

                Loader {
                    Layout.fillWidth: true
                    source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Video/VideoPanel.qml"
                }

                Repeater {
                    model: motorTelemetryViewModels

                    delegate: Loader {
                        required property var modelData

                        Layout.fillWidth: true
                        source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Motor/MotorPanel.qml"

                        onLoaded: item.viewModel = modelData
                    }
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
}
