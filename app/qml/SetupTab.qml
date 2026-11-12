import QtQuick
import QtQuick.Layouts
import GroupDecision

// What the decision is made of: options, criteria with weights, experts with influence.
GridLayout {
    id: tab

    required property DecisionController d

    columns: width > 860 ? 3 : 1
    columnSpacing: 20
    rowSpacing: 20

    // A labelled bar showing a share of the total weight
    component ShareBar: ColumnLayout {
        id: bar
        property string name: ""
        property string note: ""
        property color noteColor: Theme.muted
        property real share: 0
        Layout.fillWidth: true
        spacing: 6
        RowLayout {
            Layout.fillWidth: true
            Text { Layout.fillWidth: true; text: bar.name; color: Theme.ink; font.weight: Font.DemiBold; elide: Text.ElideRight }
            Text { text: Theme.pct(bar.share); color: Theme.ink2; font.weight: Font.DemiBold }
        }
        Rectangle {
            Layout.fillWidth: true
            height: 8
            radius: 4
            color: Theme.lineSoft
            Rectangle { height: parent.height; radius: 4; width: parent.width * bar.share; color: Theme.ultra }
        }
        Text { visible: bar.note !== ""; text: bar.note; color: bar.noteColor; font.pixelSize: 13 }
    }

    Card {
        Layout.fillWidth: true
        Layout.preferredWidth: 1
        Layout.alignment: Qt.AlignTop
        spacing: 14

        Text { text: "Options"; color: Theme.ink; font.pixelSize: 20; font.weight: Font.Bold }
        Text { text: "What the group chooses between."; color: Theme.ink2 }

        Repeater {
            model: tab.d.options
            RowLayout {
                id: optionItem
                required property var modelData
                required property int index
                spacing: 12
                Rectangle { implicitWidth: 12; implicitHeight: 12; radius: 6; color: Theme.optionColor(optionItem.index) }
                Text { text: optionItem.modelData.name; color: Theme.ink; font.weight: Font.DemiBold }
            }
        }
    }

    Card {
        Layout.fillWidth: true
        Layout.preferredWidth: 1
        Layout.alignment: Qt.AlignTop
        spacing: 16

        Text { text: "Criteria"; color: Theme.ink; font.pixelSize: 20; font.weight: Font.Bold }
        Text {
            Layout.fillWidth: true
            text: "What the options are judged on, and how much each one counts."
            color: Theme.ink2
            wrapMode: Text.WordWrap
        }

        Repeater {
            model: tab.d.criteria
            ShareBar {
                required property var modelData
                name: modelData.name
                share: modelData.share
                note: modelData.isCost ? "Lower is better" : "Higher is better"
                noteColor: modelData.isCost ? "#B5741A" : Theme.green
            }
        }
    }

    Card {
        Layout.fillWidth: true
        Layout.preferredWidth: 1
        Layout.alignment: Qt.AlignTop
        spacing: 16

        Text { text: "Experts"; color: Theme.ink; font.pixelSize: 20; font.weight: Font.Bold }
        Text {
            Layout.fillWidth: true
            text: "Who scores the options, and how much influence each person has. Agreement target: "
                  + Theme.fixed(tab.d.consensusTarget) + "."
            color: Theme.ink2
            wrapMode: Text.WordWrap
        }

        Repeater {
            model: tab.d.experts
            ShareBar {
                required property var modelData
                name: modelData.name + (modelData.isMe ? " (you)" : "")
                share: modelData.share
                note: modelData.email + (modelData.hasAccount ? "" : " · no account yet")
            }
        }
    }
}
