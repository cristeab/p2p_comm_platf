import QtQuick
import QtQuick.Controls
import ".."

Item {
    width: itercomLbl.width
    height: childrenRect.height
    IntercomLabel {
        id: itercomLbl
    }
    Image {
        id: controlImg
        anchors {
            top: itercomLbl.bottom
            topMargin: Theme.windowMargin / 2
            horizontalCenter: parent.horizontalCenter
        }
        height: Theme.logoHeight
        width: height
        source: "qrc:/img/logo.svg"
        mipmap: true
        fillMode: Image.PreserveAspectFit
    }
}
