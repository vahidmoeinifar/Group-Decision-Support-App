import QtQuick
import QtQuick.Layouts
import GroupDecision

// A row of choices where one is selected, like iOS segmented controls.
Rectangle {
    id: root

    property var model: []          // labels
    property int currentIndex: 0
    property bool dark: false
    signal activated(int index)

    implicitHeight: 42
    implicitWidth: row.implicitWidth + 8
    radius: 12
    color: dark ? "#273050" : Theme.lineSoft

    RowLayout {
        id: row
        anchors.fill: parent
        anchors.margins: 4
        spacing: 2

        Repeater {
            model: root.model

            Rectangle {
                id: segment
                required property int index
                required property string modelData
                readonly property bool selected: index === root.currentIndex

                Layout.fillWidth: true
                Layout.fillHeight: true
                implicitWidth: label.implicitWidth + 32
                radius: 9
                color: selected ? (root.dark ? Theme.ultra : "#FFFFFF") : "transparent"
                border.color: selected && !root.dark ? Theme.line : "transparent"

                Text {
                    id: label
                    anchors.centerIn: parent
                    text: segment.modelData
                    font.pixelSize: Theme.bodySize
                    font.weight: segment.selected ? Font.DemiBold : Font.Medium
                    color: root.dark ? (segment.selected ? "#FFFFFF" : Theme.sideText)
                                     : (segment.selected ? Theme.ink : Theme.ink2)
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.currentIndex = segment.index
                        root.activated(segment.index)
                    }
                }
            }
        }
    }
}
