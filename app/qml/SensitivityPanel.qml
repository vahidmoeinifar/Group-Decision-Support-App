import QtQuick
import QtQuick.Layouts
import GroupDecision

// For each criterion: which option wins as its weight moves from 0% to 100%.
// The blue part of each track is where the current winner stays first.
Card {
    id: panel

    property var info: ({})
    property var options: []

    readonly property bool ready: info.ok === true
    readonly property string winner: ready ? info.currentWinners.join(" and ") : ""
    readonly property var most: {
        if (!ready || info.mostSensitiveCriterionId === null || info.mostSensitiveCriterionId === undefined) return null
        return info.criteria.find(c => c.criterionId === info.mostSensitiveCriterionId) || null
    }

    function colorOf(optionId) {
        for (let i = 0; i < options.length; ++i)
            if (options[i].id === optionId) return Theme.optionColor(i)
        return Theme.muted
    }
    function sameSet(a, b) {
        return a.length === b.length && a.every(x => b.indexOf(x) >= 0)
    }

    Text { text: "How stable is the winner?"; color: Theme.ink; font.pixelSize: 20; font.weight: Font.Bold }

    Text {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: Theme.ink2
        text: {
            if (!panel.ready) return panel.info.error || ""
            if (!panel.most) return panel.winner + " is first, and no single weight change can change that."
            const c = panel.most
            const down = c.stableFrom > 0 ? c.currentShare - c.stableFrom : 2
            const up = c.stableTo < 1 ? c.stableTo - c.currentShare : 2
            const to = down <= up ? c.stableFrom : c.stableTo
            return panel.winner + " is in first place. The result depends most on " + c.criterion
                   + ": moving its weight from " + Theme.pct(c.currentShare) + " to " + Theme.pct(to)
                   + " would change the winner."
        }
    }

    Text {
        Layout.fillWidth: true
        visible: panel.ready
        wrapMode: Text.WordWrap
        color: Theme.muted
        font.pixelSize: 13
        text: "Using " + Theme.methodLabel(panel.info.method) + ". Each weight is moved from 0% to 100% on its own while the others keep their proportions. The small arrow marks the weight used now."
    }

    Repeater {
        model: panel.ready ? panel.info.criteria : []

        ColumnLayout {
            id: row
            required property var modelData
            readonly property bool isMost: panel.most !== null && panel.most.criterionId === modelData.criterionId
            readonly property bool whole: modelData.stableFrom === 0 && modelData.stableTo === 1

            Layout.fillWidth: true
            Layout.topMargin: 6
            spacing: 4

            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                Text { text: row.modelData.criterion; color: Theme.ink; font.weight: Font.DemiBold }
                Tag { visible: row.isMost; text: "Most sensitive"; fill: Theme.saffronSoft; textColor: "#A86A12" }
                Item { Layout.fillWidth: true }
                Text { text: "Weight now " + Theme.pct(row.modelData.currentShare); color: Theme.ink2; font.pixelSize: 13 }
            }

            Item {
                id: track
                Layout.fillWidth: true
                implicitHeight: 40

                // Pointer at the current weight
                Canvas {
                    x: track.width * row.modelData.currentShare - 6
                    y: 0
                    width: 12; height: 8
                    onPaint: {
                        const ctx = getContext("2d")
                        ctx.reset()
                        ctx.fillStyle = "#19213A"
                        ctx.beginPath(); ctx.moveTo(0, 0); ctx.lineTo(12, 0); ctx.lineTo(6, 8); ctx.closePath(); ctx.fill()
                    }
                }

                Rectangle {
                    y: 10
                    width: parent.width
                    height: 30
                    radius: 9
                    color: "transparent"
                    border.width: row.isMost ? 2 : 0
                    border.color: Theme.saffron

                    Repeater {
                        model: row.modelData.segments
                        Rectangle {
                            id: seg
                            required property var modelData
                            readonly property bool current: panel.sameSet(modelData.winnerOptionIds, panel.info.currentWinnerIds)
                            readonly property string label: modelData.winners.join(" and ")

                            x: parent.width * modelData.from + 1
                            width: Math.max(2, parent.width * (modelData.to - modelData.from) - 2)
                            y: 2
                            height: parent.height - 4
                            radius: 7
                            color: current ? Theme.ultra
                                 : modelData.winnerOptionIds.length === 1 ? panel.colorOf(modelData.winnerOptionIds[0])
                                 : Theme.line

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                x: 9
                                width: parent.width - 14
                                visible: parent.width > 60
                                text: seg.label
                                elide: Text.ElideRight
                                color: seg.current ? "#FFFFFF" : Theme.ink
                                font.pixelSize: 13
                                font.weight: Font.DemiBold
                            }
                        }
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                color: Theme.ink2
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                text: row.whole ? "Any weight keeps " + panel.winner + " first."
                                : panel.winner + " stays first between " + Theme.pct(row.modelData.stableFrom)
                                  + " and " + Theme.pct(row.modelData.stableTo) + "."
            }
        }
    }
}
