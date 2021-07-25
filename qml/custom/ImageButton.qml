import QtQuick 2.12
import QtQuick.Controls 2.12

Button {
    id: control

    property alias source: contentImage.source
    property color color: "white"
    property int radius: 6
    property alias imageScale: contentImage.scale

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
}
