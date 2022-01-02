import QtQuick
import QtQuick.Controls
import ".."

Label {
    text: "Peer to Peer<br>Communication Platform"
    wrapMode: Text.WordWrap
    width: appWin.width - 2 * Theme.windowMargin
    horizontalAlignment: Text.AlignHCenter
    font {
        pointSize: Theme.logoFontSize
        capitalization: Font.Capitalize
    }
}
