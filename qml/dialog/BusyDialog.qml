import QtQuick
import QtQuick.Controls
import "../custom"

Dialog {
    width: appWin.width
    height: appWin.height
    visible: softphone.showBusy
    standardButtons: Dialog.NoButton
    modal: true
    closePolicy: Popup.NoAutoClose
    background: Rectangle {
        anchors.fill: parent
        color: "#80000000"
    }
    contentItem: Item {
        CustomBusyIndicator {
            anchors.centerIn: parent
            height: 60
            width: height
            running: true
        }
    }
}
