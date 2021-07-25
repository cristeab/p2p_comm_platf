import QtQuick 2.12
import QtQuick.Controls 2.12

BusyIndicator {
    id: control

    property color indicatorColor: "white"
    property int indicatorHeight: 50
    property int itemHeight: 8
    property int itemCount: 8

    contentItem: Item {
        implicitWidth: control.indicatorHeight
        implicitHeight: control.indicatorHeight

        Item {
            id: item
            x: parent.width / 2 - control.indicatorHeight / 2
            y: parent.height / 2 - control.indicatorHeight / 2
            width: control.indicatorHeight
            height: control.indicatorHeight
            opacity: control.running ? 1 : 0

            Behavior on opacity {
                OpacityAnimator {
                    duration: 250
                }
            }

            RotationAnimator {
                target: item
                running: control.visible && control.running
                from: 0
                to: 360
                loops: Animation.Infinite
                duration: 2500
            }

            Repeater {
                id: repeater
                model: control.itemCount

                Rectangle {
                    x: item.width / 2 - width / 2
                    y: item.height / 2 - height / 2
                    implicitWidth: control.itemHeight / (index / 4 + 1)
                    implicitHeight: implicitWidth
                    radius: implicitWidth / 2
                    color: control.indicatorColor
                    transform: [
                        Translate {
                            y: -Math.min(item.width, item.height) * 0.5 + 5
                        },
                        Rotation {
                            angle: index / repeater.count * 360
                            origin.x: 5
                            origin.y: 5
                        }
                    ]
                }
            }
        }
    }
}
