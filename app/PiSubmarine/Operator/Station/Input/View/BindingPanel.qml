import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#13202d"
    radius: 18
    border.color: "#25465d"
    border.width: 1
    implicitWidth: 520
    implicitHeight: contentColumn.implicitHeight + 28

    TapHandler {
        acceptedButtons: Qt.LeftButton
    }

    Column {
        id: contentColumn

        width: Math.max(320, parent.width - 28)
        x: 14
        y: 14
        spacing: 10

        Label {
            text: "Input Bindings"
            color: "#dbefff"
            font.pixelSize: 22
        }

        Label {
            text: "Input Bindings"
            visible: false
        }

        Rectangle {
            width: parent.width
            color: "#0f1821"
            radius: 12
            border.color: "#203749"
            implicitHeight: tableColumn.implicitHeight + 20

            Column {
                id: tableColumn
                width: parent.width - 20
                x: 10
                y: 10
                spacing: 8

                Label { text: "Binding Table"; color: "#dbefff"; font.pixelSize: 18 }

                RowLayout {
                    width: parent.width
                    spacing: 8

                    Label {
                        text: "Path"
                        color: "#8fb4ca"
                        font.bold: true
                        Layout.preferredWidth: 120
                    }

                    Label {
                        text: "Binding"
                        color: "#8fb4ca"
                        font.bold: true
                        Layout.fillWidth: true
                    }
                }

                Repeater {
                    model: inputBindingViewModel.entries

                    delegate: Rectangle {
                        required property var modelData

                        width: tableColumn.width
                        implicitHeight: 40
                        radius: 8
                        color: "#12202c"
                        border.color: "#1d3344"

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 8

                            Label {
                                text: modelData.name
                                color: "#c0d7e7"
                                Layout.preferredWidth: 120
                                elide: Text.ElideRight
                            }

                            Label {
                                text: modelData.capturing ? "CAPTURING" : modelData.hint
                                color: modelData.capturing ? "#ffd58a" : "#f5fbff"
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                        }

                        TapHandler {
                            enabled: !inputBindingViewModel.captureInProgress
                            onTapped: inputBindingViewModel.Capture(modelData.name)
                        }
                    }
                }

                Item {
                    width: 1
                    height: 4
                }

                Button {
                    width: parent.width
                    text: inputBindingViewModel.captureInProgress ? "Cancel Capture" : "Close"
                    onClicked: {
                        if (inputBindingViewModel.captureInProgress) {
                            inputBindingViewModel.CancelCapture()
                        } else {
                            controlStatusViewModel.CloseBindingDialog()
                        }
                    }
                }
            }
        }
    }
}
