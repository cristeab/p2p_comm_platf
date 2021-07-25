import QtQuick 2.12
import QtQuick.Controls 2.12
import ".."

Item {
    id: control

    property alias stepSize: sliderControl.stepSize
    property alias from: sliderControl.from
    property alias to: sliderControl.to
    property alias value: sliderControl.value
    property string text: ""
    property string unit: ""
    property int decimalPlaces: 0
    readonly property int sliderHeight: 10
    readonly property int handleHeight: 20
    readonly property int lineWidth: 2

    width: 200
    height: textLbl.height + minLbl.height + Theme.windowMargin + control.handleHeight

    Label {
        id: textLbl
        anchors {
            bottom: sliderControl.top
            bottomMargin: Theme.windowMargin / 2
            left: parent.left
        }
        color: "white"
        font.pointSize: Theme.textFontSize
        text: control.text
    }
    Label {
        anchors {
            left: textLbl.right
            verticalCenter: textLbl.verticalCenter
        }
        color: "white"
        font.pointSize: Theme.buttonTextFontSize
        text: " - " + control.value.toFixed(control.decimalPlaces) + " " + control.unit
    }
    Slider {
        id: sliderControl
        width: parent.width
        height: control.sliderHeight

        background: Item {
            width: control.width
            height: control.sliderHeight
            Rectangle {
                anchors {
                    top: parent.top
                    left: parent.left
                }
                height: parent.height
                width: control.lineWidth
                color: "white"
            }
            Rectangle {
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }
                width: parent.width
                height: control.lineWidth
                color: "white"
            }
            Rectangle {
                anchors {
                    top: parent.top
                    right: parent.right
                }
                height: parent.height
                width: control.lineWidth
                color: "white"
            }
        }

        handle: Rectangle {
            x: - width / 2 + sliderControl.visualPosition * sliderControl.width
            y: sliderControl.topPadding - height + control.handleHeight / 2
            implicitWidth: control.handleHeight
            implicitHeight: control.handleHeight
            radius: control.handleHeight / 2
            color: sliderControl.pressed ? Qt.darker(Theme.startCallButtonColor) : Theme.startCallButtonColor
            border.color: "white"
        }
    }
    Label {
        id: minLbl
        anchors {
            top: sliderControl.bottom
            topMargin: Theme.windowMargin / 2
            left: parent.left
        }
        color: "white"
        font.pointSize: Theme.buttonTextFontSize
        text: control.from + " " + control.unit
    }
    Label {
        anchors {
            top: sliderControl.bottom
            topMargin: Theme.windowMargin / 2
            right: parent.right
        }
        color: "white"
        font.pointSize: Theme.buttonTextFontSize
        text: control.to + " " + control.unit
    }
}
