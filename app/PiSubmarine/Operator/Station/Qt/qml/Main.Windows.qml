import QtQuick
import QtQuick.Window
import org.freedesktop.gstreamer.Qt6D3D11VideoItem 1.0

Window {
    visible: true
    width: 1280
    height: 720
    color: "#05080d"
    title: "PiSubmarine Operator Station"

    GstD3D11Qt6VideoItem {
        id: videoSurface
        objectName: "videoSurface"
        anchors.fill: parent
    }
}
