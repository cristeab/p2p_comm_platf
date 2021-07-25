import QtQuick 2.0

Timer {
    id: control

    property int durationSec: -1
    property string text: ""

    function convertToString(v) {
        if (v < 10) {
            return '0' + v
        }
        return v
    }
    function showDuration() {
        const hrs = Math.floor(control.durationSec / 3600)
        const mins = Math.floor((control.durationSec % 3600) / 60)
        const sec = control.durationSec % 60
        var out = control.convertToString(mins) + ':' + control.convertToString(sec)
        if (0 < hrs) {
            out = control.convertToString(hrs) + ':' + out
        }
        control.text = out
    }

    interval: 1000
    repeat: true
    running: false
    triggeredOnStart: true
    onTriggered: {
        control.durationSec += 1
        control.showDuration()
    }
}
