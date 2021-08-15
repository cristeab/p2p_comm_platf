import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtMultimedia 5.12
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.12
import Qt.labs.platform 1.1
import "custom"
import Softphone 1.0

ApplicationWindow {
    id: appWin

    flags: Qt.MaximizeUsingFullscreenGeometryHint
    title: softphone.settings.appName + " v" + softphone.settings.appVersion
    width: appWin.minimumWidth
    height: appWin.minimumHeight
    minimumWidth: Theme.windowMinimumWidth
    minimumHeight: Theme.windowMinimumHeight
    visible: true

    signal closeIncomingCallDialog(int callId)

    function raiseWindow() {
        appWin.show()
        appWin.raise()
        appWin.requestActivate()
    }
    onClosing: {
        if (null != trayIconLoader.item) {
            trayIconLoader.item.showMessage(softphone.settings.appName,
                                            qsTr("The program will keep running in the system tray. To terminate the program, choose 'Quit' in the context menu of the system tray entry."))
        }
        if (Theme.isMacOs) {
            close.accepted = true
        } else {
            close.accepted = false
            appWin.showMinimized()
        }
    }

    QtObject {
        id: msgDlgProps
        property int contactIndex: -1
        property bool addContactFromHistory: false

        function showMessageDialog(title, message, okCancel, retry, acceptCallback, callbackArg, cancelCallback) {
            const comp = Qt.createComponent("qrc:/qml/dialog/MessageDialog.qml")
            const dlg = comp.createObject(appWin, {
                                              "visible": true,
                                              "title": title,
                                              "message": message,
                                              "okCancel": okCancel,
                                              "retry": retry,
                                              "acceptCallback": acceptCallback,
                                              "callbackArg": callbackArg,
                                              "cancelCallback": cancelCallback
                                          })
            if (null === dlg) {
                console.error("Cannot create message dialog")
            }
        }
        function showMessageConfirmationDialog(message, acceptCallback, callbackArg) {
            msgDlgProps.showMessageDialog(qsTr("Information"), message, true, false, acceptCallback, callbackArg, null)
        }
    }

    ActiveCallTimer {
        id: callDurationTimer
        running: softphone.activeCall
    }

    background: Image {
        source: "qrc:/img/background.png"
        smooth: true
        fillMode: Image.PreserveAspectCrop
    }

    StackView {
        id: tabView
        anchors.fill: parent
        initialItem: "qrc:/qml/DeviceList.qml"
        focus: true
    }

    Connections {
        target: softphone
        function onIncomingMessageDialog(callCount, callId, userName, isConf) {
            if (1 > callCount) {
                console.error("Unexpected call count " + callCount)
                return
            }
            const isAccept = (1 === callCount) || isConf
            const leftText = isAccept ? qsTr("Accept") : qsTr("Hold & Accept")
            const leftAction = isAccept ? softphone.answer : softphone.holdAndAnswer
            const comp = Qt.createComponent("qrc:/qml/dialog/IncomingCallDialog.qml")
            const dlg = comp.createObject(appWin, {
                                              "callId": callId,
                                              "text": userName,
                                              "leftButtonText": leftText,
                                              "rightButtonText": qsTr("Decline"),
                                              "actions": [leftAction, softphone.hangup],
                                              "visible": true
                                          })
            if (null === dlg) {
                console.error("Cannot create incoming call dialog")
            }
        }
        function onDisconnected(callId) {
            appWin.closeIncomingCallDialog(callId)
        }
        function onActiveCallChanged() {
            if (softphone.activeCall) {
                callDurationTimer.durationSec = -1
            } else if (2 === tabView.depth) {
                tabView.pop()
            }
        }
        function onShowMessageDialog(message, error, retry) {
            const title = error ? qsTr("Error") : qsTr("Information")
            msgDlgProps.showMessageDialog(title, message, false, retry, null, -1, null)
        }
    }

    Component {
        id: trayIconComp
        SystemTrayIcon {
            Component.onCompleted: visible = true
            icon.source: "qrc:/img/logo.png"
            onActivated: appWin.raiseWindow()
            menu: Menu {
                MenuItem {
                    text: qsTr("Quit")
                    onTriggered: Qt.quit()
                }
            }
        }
    }
    Loader {
        id: trayIconLoader
        active: !Theme.isMobile
        sourceComponent: trayIconComp
    }

    //busy indicator
    Loader {
        active: softphone.showBusy
        source: "qrc:/qml/dialog/BusyDialog.qml"
    }
}
