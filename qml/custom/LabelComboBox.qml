import QtQuick 2.12
import QtQuick.Controls 2.12
import ".."

Column {
    id: control
    property alias text: controlLabel.text
    property alias model: controlCombo.model
    property alias currentIndex: controlCombo.currentIndex
    spacing: 5
    Label {
        id: controlLabel
        width: parent.width
        elide: Text.ElideRight
        color: "white"
        font {
            italic: true
            pointSize: Theme.labelFontSize
        }
    }
    ComboBox {
        id: controlCombo
        width: parent.width
        textRole: "name"
    }
}
