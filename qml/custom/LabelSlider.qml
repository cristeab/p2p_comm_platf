import QtQuick
import QtQuick.Controls
import ".."

Column {
    id: control
    property alias text: controlLabel.text
    property alias value: controlSlider.value
    property alias from: controlSlider.from
    property alias to: controlSlider.to
    property alias stepSize: controlSlider.stepSize
    spacing: 0
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
    Slider {
        id: controlSlider
        width: parent.width
    }
}
