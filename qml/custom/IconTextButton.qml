import QtQuick
import QtQuick.Controls
import ".."

Button {
    id: control
    property alias toolTip: controlToolTip.text
    property real iconHeight: 0.5 * Theme.optionButtonHeight
    display: AbstractButton.TextUnderIcon
    height: Theme.optionButtonHeight
    width: height
    icon {
        height: control.iconHeight
        width: control.iconHeight
        color: "white"
    }
    ToolTip {
        id: controlToolTip
        visible: control.hovered && ("" !== controlToolTip.text)
    }
    background: Item {}
}
