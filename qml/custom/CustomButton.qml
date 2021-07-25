import QtQuick 2.12
import QtQuick.Controls 2.5
import ".."

Button {
    id: control
    property color backgroundColor: Theme.greenButtonColor
    property alias textColor: controlLabel.color
    height: Theme.buttonHeight
    font.pointSize: Theme.buttonFontSize
    contentItem: Label {
        id: controlLabel
        anchors.centerIn: parent
        text: control.text
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font: control.font
    }
    background: Rectangle {
        color: control.pressed?Qt.darker(control.backgroundColor):control.backgroundColor
        height: control.height
        width: control.width
        radius: Theme.buttonBackgroundRadius
    }
}
