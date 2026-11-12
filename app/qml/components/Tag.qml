import QtQuick
import GroupDecision

// A small rounded label, e.g. "Most sensitive" or "Complete".
Rectangle {
    id: root

    property string text: ""
    property color fill: Theme.lineSoft
    property color textColor: Theme.ink2

    implicitWidth: label.implicitWidth + 16
    implicitHeight: label.implicitHeight + 6
    radius: height / 2
    color: fill

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
        color: root.textColor
        font.pixelSize: Theme.smallSize
        font.weight: Font.DemiBold
    }
}
