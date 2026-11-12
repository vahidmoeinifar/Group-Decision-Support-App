import QtQuick
import QtQuick.Layouts
import GroupDecision

// The group ranking: big rank numbers and a bar for each option's score.
ColumnLayout {
    id: list

    property var ranking: []

    spacing: 16

    Repeater {
        model: list.ranking

        RowLayout {
            id: row
            required property var modelData
            readonly property bool isTop: modelData.rank === 1

            Layout.fillWidth: true
            spacing: 18

            Text {
                Layout.preferredWidth: 44
                horizontalAlignment: Text.AlignHCenter
                text: row.modelData.rank
                color: row.isTop ? Theme.saffron : "#C3CAD8"
                font.pixelSize: row.isTop ? 46 : 34
                font.weight: Font.Bold
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        Layout.fillWidth: true
                        text: row.modelData.option
                        color: Theme.ink
                        font.pixelSize: row.isTop ? 22 : 16
                        font.weight: row.isTop ? Font.Bold : Font.DemiBold
                        elide: Text.ElideRight
                    }
                    Text {
                        text: Number(row.modelData.score).toFixed(3)
                        color: Theme.ink2
                        font.weight: Font.DemiBold
                        font.family: "monospace"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: row.isTop ? 12 : 9
                    radius: height / 2
                    color: Theme.lineSoft

                    Rectangle {
                        height: parent.height
                        radius: parent.radius
                        width: Math.max(parent.height, parent.width * row.modelData.share)
                        color: row.isTop ? Theme.saffron : Theme.ultra
                        Behavior on width { NumberAnimation { duration: 350; easing.type: Easing.OutCubic } }
                    }
                }
            }
        }
    }
}
