import QtQuick
import QtQuick.Controls
import "../custom"
import ".."

Dialog {
    id: control

    property alias message: controlLabel.text
    property bool okCancel: false
    property bool retry: false
    property var acceptCallback: null
    property int callbackArg: -1
    property var cancelCallback: null

    function getStandardButtons() {
        if (control.retry) {
            return Dialog.Retry | Dialog.Cancel
        }
        return control.okCancel ? (Dialog.Ok | Dialog.Cancel) :  Dialog.Ok
    }

    implicitWidth: Theme.windowMinimumWidth - Theme.dialogMargin
    implicitHeight: 200
    x: (appWin.width-width)/2
    y: (appWin.height-height)/2
    z: 2

    background: Rectangle {
        color: Theme.backgroundColor
        radius: Theme.dialogBackgroundRadius
        border.color: Theme.dialogBorderColor
    }

    onAccepted: {
        if (null !== control.acceptCallback) {
            control.acceptCallback(control.callbackArg)
        }
        control.visible = false
        control.destroy()
    }
    onRejected: {
        if (null !== control.cancelCallback) {
            control.cancelCallback()
        }
        control.visible = false
        control.destroy()
    }
    modal: true
    closePolicy: Popup.NoAutoClose
    standardButtons: control.getStandardButtons()
    LabelToolTip {
        id: controlLabel
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        elide: Text.ElideRight
        clip: true
        font.pointSize: Theme.buttonTextFontSize
        background: Item {}
    }
}
