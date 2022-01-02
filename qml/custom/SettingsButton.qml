import QtQuick
import ".."

ImageButton {
    height: Theme.buttonHeight
    width: height
    source: "qrc:/img/settings.svg"
    color: "transparent"
    onClicked: tabView.push("qrc:/qml/Settings.qml")
}
