import QtQuick
import ".."

ImageButton {
    id: control

    signal backAction()

    height: Theme.buttonHeight
    width: height
    source: "qrc:/img/backarrow.svg"
    color: "transparent"
    onClicked: {
        tabView.pop()
        control.backAction()
    }
}
