import QtQuick 2.12
import QtQuick.Controls 2.12
import ".."

CheckBox {
    id: control
    indicator.height: 20
    indicator.width: 20
    font {
        italic: true
        pointSize: Theme.labelFontSize
    }
    contentItem: Text {
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        color: "white"
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
    }
}
