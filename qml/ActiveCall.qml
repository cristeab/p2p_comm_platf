import QtQuick 2.12
import QtQuick.Controls 2.12
import "custom"

Page {
    background: Item {}

    Label {
        anchors {
            top: parent.top
            topMargin: 2 * Theme.windowMargin
            horizontalCenter: parent.horizontalCenter
        }
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        color: "white"
        text: callDurationTimer.text
        font.pointSize: Theme.textFontSize
    }

    Row {
        anchors.centerIn: parent
        spacing: 3 * Theme.windowMargin
        IconTextButton {
            icon.source: softphone.muteMicrophone ? "qrc:/img/microphone-slash.svg" : "qrc:/img/microphone.svg"
            text: softphone.muteMicrophone ? qsTr("Unmute") : qsTr("Mute")
            toolTip: softphone.muteMicrophone ? qsTr("Enable Microphone") : qsTr("Disable Microphone")
            onClicked: softphone.muteMicrophone = !softphone.muteMicrophone
        }
        IconTextButton {
            icon.source: softphone.enableSpeaker ? "qrc:/img/volume.svg" : "qrc:/img/volume-down.svg"
            text: qsTr("Audio")
            toolTip: softphone.enableSpeaker ? qsTr("Disable Speaker") : qsTr("Enable Speaker")
            onClicked: softphone.enableSpeaker = !softphone.enableSpeaker
        }
    }

    ImageButton {
        id: callBtn
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: 4 * Theme.windowMargin
        }
        height: Theme.answerButtonHeight
        width: height
        source: "qrc:/img/hangup.svg"
        color: Theme.activeCallButtonColor
        ToolTip {
            visible: callBtn.hovered
            text: qsTr("Hangup")
        }
        onClicked: {
            if (softphone.activeCall) {
                softphone.hangupAll()
            } else if (3 === tabView.depth) {
                tabView.pop()
            }
        }
    }
}
