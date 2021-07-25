import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import "custom"

Page {
    background: Item {}

    SettingsButton {
        anchors {
            top: parent.top
            topMargin: Theme.marginFactor * Theme.windowMargin
            right: parent.right
            rightMargin: Theme.windowMargin
        }
    }
    Item {
        anchors {
            top: parent.top
            bottom: startCallBtn.bottom
        }
        width: parent.width
        Logo {
            id: logoImg
            anchors.centerIn: parent
        }
    }
    TextButton {
        id: startCallBtn
        anchors.centerIn: parent
        width: 0.6 * parent.width
        text: qsTr("Start Call")
        textColor: "white"
        backgroundColor: Theme.startCallButtonColor
        font.pointSize: Theme.buttonFontSize
        onClicked: {
            if (softphone.init()) {
                softphone.zeroConf(true)
                tabView.push("qrc:/qml/DeviceList.qml")
            }
        }
    }
}
