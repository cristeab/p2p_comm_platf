import QtQuick
import QtQuick.Controls
import ".."

Column {
    property alias text: controlLabel.text
    property alias from: controlProgress.from
    property alias to: controlProgress.to
    property alias value: controlProgress.value
    spacing: 5
    Label {
        id: controlLabel
        width: parent.width
        elide: Text.ElideRight
        color: "white"
        font {
            italic: true
            pointSize: Theme.labelFontSize
        }
    }
    ProgressBar {
        id: controlProgress
        value: 0.5
        padding: 2
        width: parent.width

        background: Rectangle {
            implicitHeight: 10
            color: "#e6e6e6"
            radius: 3
        }

        contentItem: Item {
            implicitHeight: 10

            Rectangle {
                width: controlProgress.visualPosition * parent.width
                height: parent.height
                radius: 2
                color: "#17a81a"
            }
        }
    }
}
