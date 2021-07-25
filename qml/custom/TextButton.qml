import QtQuick 2.12
import QtQuick.Controls 2.12
import ".."

Button {
    id: control

    property color textColor: "white"
    property color backgroundColor: "black"

    display: AbstractButton.TextOnly
    font.pointSize: Theme.buttonFontSize
    contentItem: Text {
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        color: control.pressed ? Qt.darker(control.textColor) : control.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: Theme.buttonBackgroundRadius
        color: control.pressed ? Qt.darker(control.backgroundColor) : control.backgroundColor
    }
}
