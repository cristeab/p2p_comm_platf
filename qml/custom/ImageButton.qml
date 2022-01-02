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

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    leftInset: 0
    rightInset: 0
    topInset: 0
    bottomInset: 0

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
    }
    ToolTip {
        id: controlToolTip
        visible: control.hovered && ("" !== control.toolTipText)
    }
}
