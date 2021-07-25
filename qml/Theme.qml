import QtQuick 2.12

pragma Singleton

QtObject {
    readonly property bool isMacOs: "osx" === Qt.platform.os
    readonly property bool isWindows: "windows" === Qt.platform.os
    readonly property bool isIos: "ios" === Qt.platform.os
    readonly property bool isAndroid: "android" === Qt.platform.os
    readonly property bool isMobile: isIos || isAndroid

    readonly property int marginFactor: isMobile ? 4 : 1

    readonly property int dialpadTextFontSize: 27
    readonly property int dialpadSubTextFontSize: 10

    readonly property real logoFontSize: 20
    readonly property real buttonFontSize: 20
    readonly property real buttonTextFontSize: 16
    readonly property real textFontSize: 18

    readonly property real windowMinimumWidth: 350
    readonly property real windowMinimumHeight: 590

    readonly property real dialogMargin: 80
    readonly property real windowMargin: 10

    readonly property real logoHeight: 50
    readonly property real busyHeight: 40
    readonly property real buttonHeight: 40
    readonly property real dialpadButtonHeight: 75
    readonly property real answerButtonHeight: 0.9 * dialpadButtonHeight
    readonly property real optionButtonHeight: dialpadButtonHeight

    readonly property real buttonBackgroundRadius: 3
    readonly property real dialogBackgroundRadius: 5

    readonly property color backgroundColor: "white"
    readonly property color dialogBorderColor: "black"
    readonly property color tabButtonColor: "#7a756b"
    readonly property color sepColor: "darkgray"
    readonly property color tabButtonColorSel: "black"
    readonly property color dialButtonBackgroundColor: "#e2e2e2"
    readonly property color dialButtonTextColor: "black"
    readonly property color callHeaderTextColor: "white"
    readonly property color callButtonColor: "#1ddd13"
    readonly property color activeCallButtonColor: "#ea0014"
    readonly property color confCallButtonColor: "#eb7414"
    readonly property color blueButtonColor: "#4fa1dc"
    readonly property color greenButtonColor: "green"
    readonly property color errorColor: "red"
    readonly property color helpTextColor: "#173683"
    readonly property color startCallButtonColor: "#FF9300"

    readonly property color incomingColor: "#86ef8a"
    readonly property color outgoingColor: "#d3d3d3"
}
