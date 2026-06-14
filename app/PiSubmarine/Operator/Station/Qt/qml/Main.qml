import QtQuick
import QtQuick.Window
import PiSubmarine.Operator.Station 1.0

Window {
    visible: true
    width: 1280
    height: 720
    color: "#05080d"
    title: "PiSubmarine Operator Station"

    VideoSurfaceItem {
        id: videoSurface
        objectName: "videoSurface"
        anchors.fill: parent
    }
}
