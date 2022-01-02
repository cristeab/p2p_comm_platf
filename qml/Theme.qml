import QtQuick
import QtQuick.Controls.Material

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
    readonly property real labelFontSize: 12

    readonly property real windowMinimumWidth: 350
    readonly property real windowMinimumHeight: 590

    readonly property real dialogMargin: 80
    readonly property real windowMargin: 10

    readonly property real logoHeight: 50
    readonly property real busyHeight: 40
    readonly property real buttonHeight: 25
    readonly property real dialpadButtonHeight: 75
    readonly property real answerButtonHeight: 0.9 * dialpadButtonHeight
    readonly property real optionButtonHeight: dialpadButtonHeight

    readonly property real buttonBackgroundRadius: 3
    readonly property real dialogBackgroundRadius: 5

    readonly property color backgroundColor: Material.background
    readonly property color dialogBorderColor: Material.color(Material.Brown)
    readonly property color tabButtonColor: Material.color(Material.Brown)
    readonly property color sepColor: Material.color(Material.BlueGrey)
    readonly property color tabButtonColorSel: Material.background
    readonly property color dialButtonBackgroundColor: Material.color(Material.Grey)
    readonly property color dialButtonTextColor: Material.background
    readonly property color callHeaderTextColor: Material.foreground
    readonly property color callButtonColor: Material.color(Material.Green)
    readonly property color activeCallButtonColor: Material.color(Material.Red)
    readonly property color confCallButtonColor: Material.color(Material.Orange)
    readonly property color blueButtonColor: Material.color(Material.Blue)
    readonly property color greenButtonColor: Material.color(Material.Green)
    readonly property color errorColor: Material.color(Material.Red)
    readonly property color helpTextColor: Material.color(Material.Indigo)
    readonly property color startCallButtonColor: Material.color(Material.Purple)

    readonly property color incomingColor: Material.color(Material.LightGreen)
    readonly property color outgoingColor: Material.color(Material.BlueGrey)
}
