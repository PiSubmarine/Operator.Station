import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 1440
    height: 860
    color: "#08111a"
    title: "PiSubmarine Operator Station"

    Item {
        anchors.fill: parent

        Rectangle {
            anchors.fill: parent
            anchors.margins: 16
            color: "#03070b"
            radius: 20
            border.color: "#163348"
            clip: true

            Item {
                id: videoSurfaceHost
                objectName: "videoSurfaceHost"
                anchors.fill: parent
                clip: true
            }

            Loader {
                z: 2
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 12
                sourceComponent: Row {
                    spacing: 10

                    Loader {
                        source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Time/TimePanel.qml"
                    }

                    Loader {
                        source: "qrc:/PiSubmarine/Operator/Station/Control/View/StatusPanel.qml"
                    }
                }
            }

            Loader {
                z: 3
                anchors.fill: parent
                anchors.margins: 24
                source: "qrc:/PiSubmarine/Operator/Station/View/OverlayContainer.qml"

                onLoaded: {
                    item.overlayViewModels = videoOverlayViewModels
                }
            }

            Column {
                z: 3
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12

                Loader {
                    source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Battery/BatteryPanel.qml"
                }

                Grid {
                    columns: 2
                    rows: 2
                    rowSpacing: 8
                    columnSpacing: 8

                    Repeater {
                        model: motorTelemetryViewModels

                        delegate: Loader {
                            required property var modelData

                            source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Motor/MotorPanel.qml"

                            onLoaded: item.viewModel = modelData
                        }
                    }
                }

                Rectangle {
                    color: "#f0091823"
                    radius: 16
                    border.color: "#2d617a"
                    border.width: 1
                    implicitWidth: 168
                    implicitHeight: 104

                    Column {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 6

                        Loader {
                            source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Proximity/ProximityOverlayRow.qml"
                        }

                        Loader {
                            source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Depth/DepthOverlayRow.qml"
                        }

                        Loader {
                            source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Ballast/BallastOverlayRow.qml"
                        }
                    }
                }

                Loader {
                    source: "qrc:/PiSubmarine/Operator/Station/Telemetry/View/Lamp/LampOverlayPanel.qml"
                }
            }

            Column {
                z: 3
                anchors.right: parent.right
                anchors.rightMargin: 24
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12

                Loader {
                    source: "qrc:/PiSubmarine/Operator/Station/Control/View/CameraControlOverlay.qml"
                }

                Loader {
                    source: "qrc:/PiSubmarine/Operator/Station/Control/View/ModeControlOverlay.qml"
                }

                Loader {
                    source: "qrc:/PiSubmarine/Operator/Station/Control/View/VerticalControlOverlay.qml"
                }

                Loader {
                    source: "qrc:/PiSubmarine/Operator/Station/Control/View/LampControlOverlay.qml"
                }
            }

            Rectangle {
                z: 4
                anchors.fill: parent
                color: "#02070dcc"
                visible: controlStatusViewModel.bindingDialogVisible

                TapHandler {
                    onTapped: controlStatusViewModel.CloseBindingDialog()
                }

                Loader {
                    anchors.centerIn: parent
                    width: Math.min(parent.width - 64, 640)
                    source: "qrc:/PiSubmarine/Operator/Station/Input/View/BindingPanel.qml"
                }

                TapHandler {
                    acceptedButtons: Qt.LeftButton
                    gesturePolicy: TapHandler.ReleaseWithinBounds
                    onTapped: function(eventPoint) { eventPoint.accepted = false }
                }
            }
        }
    }
}
