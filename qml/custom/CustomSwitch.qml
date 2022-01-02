import QtQuick
import QtQuick.Controls
import ".."

Item {
    id: control

    property alias checked: controlSwitch.checked
    property alias mainText: controlMainLbl.text
    property alias leftText: controlLeftLbl.text
    property alias rightText: controlRightLbl.text

    height: controlMainLbl.height + controlLeftLbl.height
    Label {
        id: controlMainLbl
        anchors {
            top: parent.top
            left: parent.left
        }
        width: parent.width
        color: "white"
        font.pointSize: Theme.textFontSize
    }
    Label {
        id: controlLeftLbl
        anchors {
            verticalCenter: controlSwitch.verticalCenter
            left: parent.left
        }
        color: "white"
        font.pointSize: Theme.buttonTextFontSize
    }
    Switch {
        id: controlSwitch

        readonly property int indicatorHeight: 26
        width: 48
        height: indicatorHeight

        anchors {
            top: controlMainLbl.bottom
            left: controlLeftLbl.right
            leftMargin: Theme.windowMargin
        }
        indicator: Rectangle {
            implicitWidth: controlSwitch.width
            implicitHeight: controlSwitch.indicatorHeight
            x: controlSwitch.leftPadding
            y: parent.height / 2 - height / 2
            radius: controlSwitch.indicatorHeight / 2
            color: "white"
            border.color: "white"

            Rectangle {
                x: controlSwitch.checked ? parent.width - width : 0
                width: controlSwitch.indicatorHeight
                height: controlSwitch.indicatorHeight
                radius: controlSwitch.indicatorHeight / 2
                color: controlSwitch.down ? Qt.darker(Theme.startCallButtonColor) : Theme.startCallButtonColor
                border.color: "white"
            }
        }
        contentItem: Item {}
    }
    Label {
        id: controlRightLbl
        anchors {
            verticalCenter: controlSwitch.verticalCenter
            left: controlSwitch.right
            leftMargin: 2 * Theme.windowMargin
        }
        color: "white"
        font.pointSize: Theme.buttonTextFontSize
    }
}
