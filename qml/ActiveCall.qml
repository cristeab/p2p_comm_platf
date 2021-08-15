import QtQuick 2.12
import QtQuick.Controls 2.12
import "custom"

Page {
    background: Item {}

    IntercomLabel {
        id: currentDevLbl
        anchors {
            top: parent.top
            topMargin: 4 * Theme.windowMargin
            horizontalCenter: parent.horizontalCenter
        }
        font.capitalization: Font.MixedCase
        text: softphone.currentDeviceName
    }
    Label {
        anchors {
            top: currentDevLbl.bottom
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
            icon.source: softphone.muteSpeakers ? "qrc:/img/volume-down.svg" : "qrc:/img/volume.svg"
            text: qsTr("Audio")
            toolTip: softphone.muteSpeakers ? qsTr("Enable Speaker") : qsTr("Disable Speaker")
            onClicked: softphone.muteSpeakers = !softphone.muteSpeakers
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
            softphone.hangupAll()
            tabView.pop()
        }
    }
}
