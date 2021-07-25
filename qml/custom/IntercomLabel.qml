import QtQuick 2.12
import QtQuick.Controls 2.12
import ".."

Label {
    text: "Peer to Peer<br>Communication<br>Platform"
    wrapMode: Text.WordWrap
    width: appWin.width - 2 * Theme.windowMargin
    horizontalAlignment: Text.AlignHCenter
    color: "white"
    font {
        pointSize: Theme.logoFontSize
        capitalization: Font.Capitalize
    }
}
