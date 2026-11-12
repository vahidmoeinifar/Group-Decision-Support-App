import QtQuick
import QtQuick.Layouts
import GroupDecision

// Shows the current round, how many experts have finished, and agreement against the target.
// The organizer can start the next round from here.
Rectangle {
    id: bar

    required property DecisionController d
    signal reviewScores()

    readonly property var result: d.revision >= 0 ? d.result("Topsis") : ({})
    readonly property var consensus: result.ok ? result.consensus : null
    readonly property bool hasValue: consensus !== null && consensus !== undefined
    readonly property bool targetMet: hasValue && consensus >= d.consensusTarget
    readonly property int complete: d.completeCount

    implicitHeight: row.implicitHeight + 36
    radius: Theme.radius
    color: targetMet ? Theme.greenSoft : hasValue ? Theme.saffronSoft : Theme.paper
    border.color: targetMet ? "#BFE3D0" : hasValue ? "#F0D6A8" : Theme.line

    RowLayout {
        id: row
        anchors.fill: parent
        anchors.leftMargin: 24
        anchors.rightMargin: 22
        spacing: 26

        ColumnLayout {
            spacing: -2
            Text { text: "Round"; color: Theme.muted; font.pixelSize: 13; Layout.alignment: Qt.AlignHCenter }
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: bar.d.currentRound
                color: bar.targetMet ? Theme.green : bar.hasValue ? "#B5741A" : Theme.ink
                font.pixelSize: 38
                font.weight: Font.Bold
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 3
            Text {
                Layout.fillWidth: true
                text: bar.complete + " of " + bar.d.experts.length + " experts have finished scoring."
                color: Theme.ink
                font.weight: Font.DemiBold
                wrapMode: Text.WordWrap
            }
            Text {
                Layout.fillWidth: true
                color: Theme.ink2
                wrapMode: Text.WordWrap
                text: {
                    if (!bar.hasValue)
                        return "Agreement appears when at least two experts have finished."
                    const value = Theme.fixed(bar.consensus)
                    const target = Theme.fixed(bar.d.consensusTarget)
                    if (bar.targetMet)
                        return "Agreement is " + value + ", which meets the target of " + target + "."
                    return "Agreement is " + value + ", below the target of " + target
                           + (bar.d.isOwner ? ". Start another round so experts can revise their scores."
                                            : ". The organizer may start another round.")
                }
            }
        }

        AppButton {
            visible: bar.d.isOwner
            kind: "primary"
            enabled: bar.complete >= 2
            text: "Start round " + (bar.d.currentRound + 1)
            onClicked: confirm.open()
        }

        AppButton {
            visible: !bar.d.isOwner && bar.d.myExpertId >= 0
            text: "Review my scores"
            onClicked: bar.reviewScores()
        }
    }

    ConfirmDialog {
        id: confirm
        title: "Start round " + (bar.d.currentRound + 1) + "?"
        message: "Every expert's current scores are copied into the new round. Experts then see where they differ from the group and can revise. Results of round "
                 + bar.d.currentRound + " stay in the history."
        confirmText: "Start round " + (bar.d.currentRound + 1)
        onAccepted: bar.d.startNextRound()
    }
}
