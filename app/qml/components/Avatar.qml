import QtQuick
import GroupDecision

// A round badge with someone's initials.
Rectangle {
    id: root

    property string initials: ""
    property color fill: Theme.saffron
    property color textColor: Theme.ink

    implicitWidth: 38
    implicitHeight: 38
    radius: width / 2
    color: fill

    Text {
        anchors.centerIn: parent
        text: root.initials
        color: root.textColor
        font.pixelSize: Math.round(root.width * 0.34)
        font.weight: Font.Bold
    }
}
