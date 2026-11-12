import QtQuick
import QtQuick.Layouts
import GroupDecision

// A friendly message shown when there is nothing to display yet.
ColumnLayout {
    id: root

    property string title: ""
    property string text: ""

    spacing: 6

    Text {
        Layout.fillWidth: true
        text: root.title
        color: Theme.ink
        font.pixelSize: 17
        font.weight: Font.DemiBold
        wrapMode: Text.WordWrap
    }
    Text {
        Layout.fillWidth: true
        visible: root.text !== ""
        text: root.text
        color: Theme.ink2
        font.pixelSize: Theme.bodySize
        wrapMode: Text.WordWrap
        lineHeight: 1.25
    }
}
