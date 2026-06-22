import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/PiSubmarine/Operator/Station/View/Theme.js" as Theme

Rectangle {
    color: Theme.panelBackground
    radius: Theme.panelRadius
    border.color: Theme.panelBorder
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

        Rectangle {
            width: parent.width
            color: Theme.panelBackground
            radius: 12
            border.color: Theme.panelBorder
            implicitHeight: tableColumn.implicitHeight + 20

            Column {
                id: tableColumn
                width: parent.width - 20
                x: 10
                y: 10
                spacing: 8

                RowLayout {
                    width: parent.width
                    spacing: 8

                    Label {
                        text: "Path"
                        color: Theme.textColorH2
                        font.bold: true
                        Layout.preferredWidth: 120
                        font.pixelSize: Theme.textFontSizeH2
                    }

                    Label {
                        text: "Binding"
                        color: Theme.textColorH2
                        font.bold: true
                        Layout.fillWidth: true
                        font.pixelSize: Theme.textFontSizeH2
                    }
                }

                Repeater {
                    model: inputBindingViewModel.entries

                    delegate: Rectangle {
                        required property var modelData

                        width: tableColumn.width
                        implicitHeight: 40
                        radius: 8
                        color: Theme.panelBackground
                        border.color: Theme.panelBorder

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 8

                            Label {
                                text: modelData.name
                                color: Theme.textColorH4
                                Layout.preferredWidth: 120
                                elide: Text.ElideRight
                                font.pixelSize: Theme.textFontSizeH4
                            }

                            Label {
                                text: modelData.capturing ? "CAPTURING" : modelData.hint
                                color: modelData.capturing ? Theme.textWarning : Theme.textColorH4
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                                font.pixelSize: Theme.textFontSizeH4
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

                Rectangle {
                    width: parent.width
                    implicitHeight: 32
                    radius: Theme.buttonRadius
                    border.width: 1
                    border.color: actionTapHandler.pressed ? Theme.buttonPressedBorder : Theme.panelBorder
                    color: actionTapHandler.pressed
                        ? Theme.buttonPressedBackground
                        : (actionHoverHandler.hovered ? Theme.buttonHoverBackground : Theme.panelBackground)

                    Label {
                        anchors.centerIn: parent
                        text: inputBindingViewModel.captureInProgress ? "Cancel Capture" : "Close"
                        color: Theme.textColorH4
                        font.pixelSize: Theme.textFontSizeH4
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    HoverHandler {
                        id: actionHoverHandler
                    }

                    TapHandler {
                        id: actionTapHandler
                        onTapped: {
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
}
