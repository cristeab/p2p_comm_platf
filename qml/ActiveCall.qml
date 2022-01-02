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
    }
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
        text: callDurationTimer.text
        font.pointSize: Theme.textFontSize
    }

    Row {
        anchors.centerIn: parent
        spacing: 0
        IconTextButton {
            icon.source: softphone.muteMicrophone ? "qrc:/img/microphone-slash.svg" : "qrc:/img/microphone.svg"
            text: qsTr("Microphone")
            width: 1.5 * Theme.optionButtonHeight
            toolTip: softphone.muteMicrophone ? qsTr("Enable") : qsTr("Disable")
            onClicked: softphone.muteMicrophone = !softphone.muteMicrophone
        }
        IconTextButton {
            icon.source: softphone.muteSpeakers ? "qrc:/img/volume-down.svg" : "qrc:/img/volume.svg"
            text: qsTr("Speaker")
            width: 1.5 * Theme.optionButtonHeight
            toolTip: softphone.muteSpeakers ? qsTr("Enable") : qsTr("Disable")
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
        imageScale: 0.8
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
