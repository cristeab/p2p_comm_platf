import QtQuick
import QtQuick.Controls

Button {
    id: control

    property alias source: contentImage.source
    property color color: "white"
    property int radius: 6
    property alias imageScale: contentImage.scale
    property alias toolTipText: controlToolTip.text

    property bool isPressed: control.pressed || control.checked

    background: Rectangle {
        color: control.isPressed ? Qt.darker(control.color) : control.color
        height: control.height
        width: control.width
        radius: height / 2
    }
    contentItem: Image {
        id: contentImage
        asynchronous: true
        fillMode: Image.PreserveAspectFit
        height: 0.5*parent.height
        width: height
        mipmap: true
        scale: control.isPressed ? 0.9 : 1
    }
    ToolTip {
        id: controlToolTip
        visible: control.hovered
    }
}
