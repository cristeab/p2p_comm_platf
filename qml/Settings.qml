import QtQuick 2.12
import QtQuick.Controls 2.12
import Settings 1.0
import "custom"

Page {
    background: Item {}

    BackButton {
        id: backBtn
        anchors {
            top: parent.top
            topMargin: Theme.marginFactor * Theme.windowMargin
            left: parent.left
            leftMargin: Theme.windowMargin
        }
        onBackAction: softphone.setupAudioCodecParam()
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
        color: "white"
        text: qsTr("Advanced Settings")
        font.pointSize: Theme.textFontSize
    }

    Column {
        id: settCol
        anchors {
            top: titleLbl.bottom
            topMargin: 6 * Theme.windowMargin
            left: parent.left
            leftMargin: 2 * Theme.windowMargin
            right: parent.right
            rightMargin: 2 * Theme.windowMargin
        }
        spacing: Theme.windowMargin
        CustomSlider {
            text: qsTr("Bitrate")
            width: parent.width
            unit: "kb/s"
            stepSize: 1
            from: Settings.CODEC_OPUS_DEFAULT_BIT_RATE_KBPS
            to: Settings.CODEC_OPUS_MAX_BIT_RATE_KBPS
            value: softphone.settings.bitrateKbps
            onValueChanged: {
                if ((Settings.CODEC_OPUS_DEFAULT_BIT_RATE_KBPS !== value) &&
                        (value < Settings.CODEC_OPUS_MIN_BIT_RATE_KBPS)) {
                    softphone.settings.bitrateKbps = Settings.CODEC_OPUS_MIN_BIT_RATE_KBPS
                } else {
                    softphone.settings.bitrateKbps = value
                }
            }
        }
        CustomSlider {
            text: qsTr("Sampling Rate")
            width: parent.width
            unit: "kHz"
            stepSize: 4
            from: Settings.CODEC_OPUS_MIN_SAMPLE_RATE_KHZ
            to: Settings.CODEC_OPUS_MAX_SAMPLE_RATE_KHZ
            value: softphone.settings.sampRateKhz
            onValueChanged: {
                if ((Settings.CODEC_OPUS_SAMPLE_RATE_1_KHZ < value) &&
                        (value < Settings.CODEC_OPUS_SAMPLE_RATE_2_KHZ)) {
                    softphone.settings.sampRateKhz = Settings.CODEC_OPUS_SAMPLE_RATE_1_KHZ
                } else if ((Settings.CODEC_OPUS_SAMPLE_RATE_2_KHZ <= value) &&
                           (value <= (Settings.CODEC_OPUS_SAMPLE_RATE_2_KHZ + 12))) {
                    softphone.settings.sampRateKhz = Settings.CODEC_OPUS_SAMPLE_RATE_2_KHZ
                } else if ((Settings.CODEC_OPUS_SAMPLE_RATE_2_KHZ + 12) < value) {
                    softphone.settings.sampRateKhz = Settings.CODEC_OPUS_MAX_SAMPLE_RATE_KHZ
                } else {
                    softphone.settings.sampRateKhz = value
                }
            }
        }
        CustomSlider {
            text: qsTr("Frame Size")
            width: parent.width
            unit: "ms"
            stepSize: 0.1
            from: softphone.settings.frameSizeMsMin
            to: softphone.settings.frameSizeMsMax
            value: softphone.settings.frameSizeMs
            decimalPlaces: 1
            onValueChanged: softphone.settings.frameSizeMs = value
        }
        CustomSlider {
            text: qsTr("Lost Connection Timeout")
            width: parent.width
            unit: "sec"
            stepSize: 1
            from: softphone.settings.lostConnTimeoutSecMin
            to: softphone.settings.lostConnTimeoutSecMax
            value: softphone.settings.lostConnTimeoutSec
            onValueChanged: softphone.settings.lostConnTimeoutSec = value
        }
    }
    CustomSwitch {
        anchors {
            top: settCol.bottom
            topMargin: -2 * Theme.windowMargin
            left: settCol.left
            right: settCol.right
        }
        mainText: qsTr("Bit Rate")
        leftText: qsTr("Constant")
        rightText: qsTr("Variable")
        checked: softphone.settings.isVariableBitRate
        onCheckedChanged: softphone.settings.isVariableBitRate = checked
    }
}
