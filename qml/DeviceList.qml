import QtQuick
import QtQuick.Controls
import "custom"

Page {
    background: Item {}

    SettingsButton {
        anchors {
            top: parent.top
            topMargin: Theme.windowMargin
            right: parent.right
            rightMargin: Theme.windowMargin
        }
        toolTipText: qsTr("Settings")
    }   
    Logo {
        id: logoFrame
        anchors {
            top: parent.top
            topMargin: Theme.windowMargin
        }
        width: parent.width
    }

    Column {
        id: headerCol
        anchors {
            top: logoFrame.bottom
            topMargin: Theme.windowMargin
            left: parent.left
            leftMargin: 3 * Theme.windowMargin
            right: parent.right
            rightMargin: 3 * Theme.windowMargin
        }
        spacing: Theme.windowMargin
        width: parent.width
        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Looking for other devices.")
            font.pointSize: Theme.textFontSize
            wrapMode: Text.WordWrap
        }
        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Ensure other devices are on the same network and have this app.")
            font.pointSize: Theme.textFontSize
            wrapMode: Text.WordWrap
        }
    }
    ListView {
        id: devList
        anchors {
            top: headerCol.bottom
            topMargin: 2 * Theme.windowMargin
            bottom: parent.bottom
            bottomMargin: 3 * Theme.windowMargin
            left: headerCol.left
            right: headerCol.right
        }
        model: softphone.deviceList
        clip: true
        snapMode: ListView.SnapToItem
        boundsBehavior: Flickable.StopAtBounds
        delegate: ItemDelegate {
            text: modelData
            width: ListView.view.width
            contentItem: Text {
                text: parent.text
                width: parent.width
                font.pointSize: Theme.textFontSize
                elide: Text.ElideRight
                color:  "black"
                leftPadding: Theme.windowMargin
                rightPadding: Theme.windowMargin
                verticalAlignment: Text.AlignVCenter
            }
            background: Item {
                width: parent.width
                Rectangle {
                    anchors.bottom: parent.bottom
                    color: Theme.sepColor
                    width: parent.width
                    height: 1
                }
            }
            onClicked: {
                console.log("Selected " + index)
                devList.currentIndex = index
                if (softphone.makeCall(index)) {
                    tabView.push("qrc:/qml/ActiveCall.qml")
                }
            }
        }
        highlight: Rectangle {
            color: "lightsteelblue"
            radius: Theme.dialogBackgroundRadius
        }
        Rectangle {
            z: -1
            anchors.fill: parent
            color: "#80FFFFFF"
            radius: Theme.dialogBackgroundRadius
        }
    }

    Component {
        id: busyComp
        CustomBusyIndicator {
            indicatorHeight: Theme.busyHeight
            height: indicatorHeight
            width: height
            running: true
        }
    }
    Loader {
        active: 0 === devList.count
        sourceComponent: busyComp
        anchors.centerIn: devList
    }
}
