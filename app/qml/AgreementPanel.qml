import QtQuick
import QtQuick.Layouts
import GroupDecision

// Agreement level of the group (-1..1), the target, and each expert against the group.
Card {
    id: panel

    property var result: ({})
    property real target: 0.8

    readonly property var consensus: result.ok ? result.consensus : null
    readonly property bool hasValue: consensus !== null && consensus !== undefined
    function position(x) { return (x + 1) / 2 }   // -1..1 to 0..1

    spacing: 10

    Text { text: "Agreement"; color: Theme.ink; font.weight: Font.DemiBold }

    RowLayout {
        spacing: 12
        Text {
            text: panel.hasValue ? Theme.fixed(panel.consensus) : "–"
            color: Theme.ink
            font.pixelSize: 42
            font.weight: Font.Bold
        }
        Text {
            Layout.alignment: Qt.AlignBottom
            Layout.bottomMargin: 8
            text: Theme.agreementLabel(panel.hasValue ? panel.consensus : null)
            color: Theme.ink
            font.weight: Font.DemiBold
        }
    }

    // Scale from "opposite" (-1) to "same" (+1)
    Item {
        Layout.fillWidth: true
        implicitHeight: 14

        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width
            height: 6
            radius: 3
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "#EDB8C1" }
                GradientStop { position: 0.5; color: "#EBDDB0" }
                GradientStop { position: 1.0; color: "#8ED1B0" }
            }
        }
        Rectangle {   // target
            x: parent.width * panel.position(panel.target) - 1
            anchors.verticalCenter: parent.verticalCenter
            width: 2; height: 14
            color: Theme.green
        }
        Rectangle {   // current value
            visible: panel.hasValue
            x: parent.width * panel.position(panel.hasValue ? panel.consensus : 0) - 2
            anchors.verticalCenter: parent.verticalCenter
            width: 4; height: 20; radius: 2
            color: Theme.ink
        }
    }

    Item {
        Layout.fillWidth: true
        implicitHeight: 16
        Text { text: "Opposite"; color: Theme.muted; font.pixelSize: Theme.smallSize }
        Text {
            x: Math.min(parent.width - width - 40, Math.max(60, parent.width * panel.position(panel.target) - width / 2))
            text: "Target " + Theme.fixed(panel.target)
            color: Theme.green
            font.pixelSize: Theme.smallSize
        }
        Text { anchors.right: parent.right; text: "Same"; color: Theme.muted; font.pixelSize: Theme.smallSize }
    }

    Text {
        Layout.topMargin: 8
        visible: panel.result.ok && panel.result.experts.length > 0
        text: "Each expert against the group"
        color: Theme.ink
        font.weight: Font.DemiBold
    }

    Repeater {
        model: panel.result.ok ? panel.result.experts : []

        ColumnLayout {
            id: expertRow
            required property var modelData
            Layout.fillWidth: true
            spacing: 5

            RowLayout {
                Layout.fillWidth: true
                Text { Layout.fillWidth: true; text: expertRow.modelData.expert; color: Theme.ink2; elide: Text.ElideRight }
                Text { text: Theme.fixed(expertRow.modelData.agreement); color: Theme.ink; font.weight: Font.Bold }
            }

            // Bar grows right from the middle for agreement, left for disagreement
            Item {
                Layout.fillWidth: true
                implicitHeight: 6
                Rectangle { anchors.fill: parent; radius: 3; color: Theme.lineSoft }
                Rectangle { x: parent.width / 2; width: 1; height: parent.height; color: "#AEB6C6" }
                Rectangle {
                    readonly property real value: expertRow.modelData.agreement
                    height: parent.height
                    radius: 3
                    x: value >= 0 ? parent.width / 2 : parent.width / 2 * (1 + value)
                    width: parent.width / 2 * Math.abs(value)
                    color: value >= panel.target ? Theme.green : value >= 0 ? Theme.saffron : Theme.red
                }
            }
        }
    }

    Text {
        Layout.fillWidth: true
        Layout.topMargin: 4
        visible: panel.result.ok && panel.result.skipped.length > 0
        text: panel.result.ok ? "Not included until their scores are complete: " + panel.result.skipped.join(", ") + "." : ""
        color: Theme.muted
        font.pixelSize: 13
        wrapMode: Text.WordWrap
    }
}
