import QtQuick
import QtQuick.Controls
import Settings 1.0
import "custom"

Page {
    background: Item {}

    BackButton {
        id: backBtn
        toolTipText: qsTr("Back")
        anchors {
            top: parent.top
            topMargin: Theme.marginFactor * Theme.windowMargin
            left: parent.left
            leftMargin: Theme.windowMargin
        }
    }
    Label {
        id: titleLbl
        anchors {
            top: backBtn.bottom
            topMargin: Theme.windowMargin
            left: parent.left
            leftMargin: Theme.windowMargin
            right: parent.right
            rightMargin: Theme.windowMargin
        }
        horizontalAlignment: Text.AlignHCenter
        text: qsTr("Settings")
        font.pointSize: Theme.textFontSize
    }

    Flickable {
        anchors {
            top: titleLbl.bottom
            topMargin: 2 * Theme.windowMargin
            bottom: parent.bottom
            bottomMargin: Theme.windowMargin / 2
            left: parent.left
            leftMargin: Theme.windowMargin
            right: parent.right
            rightMargin: Theme.windowMargin
        }
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: devicesLayout.width
        contentHeight: devicesLayout.height
        clip: true

        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
        ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }
        Column {
            id: devicesLayout
            spacing: 10
            LabelComboBox {
                id: callOutputSrc
                text: qsTr("Speaker Source")
                width: appWin.width - 2 * Theme.windowMargin
                model: softphone.outputAudioDevices
                currentIndex: softphone.settings.outputAudioModelIndex
                onCurrentIndexChanged: softphone.settings.outputAudioModelIndex = currentIndex
            }
            LabelSlider {
                text: qsTr("Speaker Volume")
                width: callOutputSrc.width
                from: 0
                to: 5
                stepSize: 0.1
                value: softphone.settings.speakersVolume
                onValueChanged: softphone.settings.speakersVolume = value
            }
            LabelProgressBar {
                visible: softphone.activeCall
                text: qsTr("Speaker Level")
                width: callOutputSrc.width
                from: 0
                to: 255
                value: softphone.rxLevel
            }
            LabelComboBox {
                id: inputAudioDevs
                text: qsTr("Microphone Source")
                width: callOutputSrc.width
                model: softphone.inputAudioDevices
                currentIndex: softphone.settings.inputAudioModelIndex
                onCurrentIndexChanged: softphone.settings.inputAudioModelIndex = inputAudioDevs.currentIndex
            }
            LabelSlider {
                text: qsTr("Microphone Volume")
                width: callOutputSrc.width
                from: 0
                to: 5
                stepSize: 0.1
                value: softphone.settings.microphoneVolume
                onValueChanged: softphone.settings.microphoneVolume = value
            }
            LabelProgressBar {
                visible: softphone.activeCall
                text: qsTr("Microphone Level")
                width: callOutputSrc.width
                from: 0
                to: 255
                value: softphone.txLevel
            }
            CustomCheckBox {
                id: alwaysOnTopCheck
                visible: !Theme.isMobile
                checked: softphone.settings.alwaysOnTop
                onCheckedChanged: softphone.settings.alwaysOnTop = checked
                text: qsTr("Always on Top")
            }
        } // Column
    }
}
