import QtQuick
import QtQuick.Layouts
import GroupDecision

// Line chart of agreement per round, with the target as a dashed line.
Card {
    id: card

    property var rounds: []    // [{round, consensus, winners, expertCount, isCurrent}]
    property real target: 0.8

    readonly property var points: rounds.filter(r => r.consensus !== null && r.consensus !== undefined)
    readonly property real minY: points.some(p => p.consensus < 0) ? -1 : 0

    Text { text: "Agreement by round"; color: Theme.ink; font.pixelSize: 20; font.weight: Font.Bold }

    Text {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: Theme.ink2
        text: {
            const p = card.points
            if (p.length === 0) return "No round has enough scores yet."
            if (p.length === 1) return "Round " + p[0].round + " has agreement " + Theme.fixed(p[0].consensus)
                                       + ". Start another round to see how it changes."
            const a = p[0], b = p[p.length - 1]
            return "Agreement went from " + Theme.fixed(a.consensus) + " in round " + a.round + " to "
                   + Theme.fixed(b.consensus) + " in round " + b.round + "."
        }
    }

    Canvas {
        id: canvas
        Layout.fillWidth: true
        Layout.topMargin: 6
        implicitHeight: 190
        visible: card.points.length > 0

        onWidthChanged: requestPaint()
        Connections {
            target: card
            function onRoundsChanged() { canvas.requestPaint() }
        }

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            const left = 34, right = width - 22, top = 18, bottom = height - 26
            const n = Math.max(1, card.rounds.length)
            const x = r => n === 1 ? (left + right) / 2 : left + (r - 1) * (right - left) / (n - 1)
            const y = v => bottom - (v - card.minY) / (1 - card.minY) * (bottom - top)

            // Grid and axis labels
            ctx.font = "11px sans-serif"
            ctx.fillStyle = "#7A849B"
            ctx.strokeStyle = "#E3E8EF"
            ctx.lineWidth = 1
            const ticks = card.minY < 0 ? [-1, 0, 1] : [0, 0.5, 1]
            for (const t of ticks) {
                ctx.beginPath(); ctx.moveTo(left, y(t)); ctx.lineTo(right, y(t)); ctx.stroke()
                ctx.fillText(t.toFixed(1), 0, y(t) + 4)
            }
            for (const r of card.rounds) {
                const label = "Round " + r.round
                const w = ctx.measureText(label).width
                ctx.fillText(label, Math.max(0, Math.min(width - w, x(r.round) - w / 2)), height - 6)
            }

            // Target
            ctx.strokeStyle = "#1F8A5B"
            ctx.setLineDash([5, 4])
            ctx.beginPath(); ctx.moveTo(left, y(card.target)); ctx.lineTo(right, y(card.target)); ctx.stroke()
            ctx.setLineDash([])
            ctx.fillStyle = "#1F8A5B"
            ctx.font = "bold 11px sans-serif"
            ctx.fillText("Target " + card.target.toFixed(2), left + 6, y(card.target) - 6)

            // Line and points
            const p = card.points
            ctx.strokeStyle = "#3346D3"
            ctx.lineWidth = 2.5
            ctx.beginPath()
            p.forEach((pt, i) => i === 0 ? ctx.moveTo(x(pt.round), y(pt.consensus)) : ctx.lineTo(x(pt.round), y(pt.consensus)))
            ctx.stroke()

            ctx.font = "bold 12px sans-serif"
            for (const pt of p) {
                const met = pt.consensus >= card.target
                ctx.beginPath()
                ctx.arc(x(pt.round), y(pt.consensus), 6, 0, Math.PI * 2)
                ctx.fillStyle = met ? "#1F8A5B" : "#FFFFFF"
                ctx.fill()
                ctx.strokeStyle = met ? "#1F8A5B" : "#3346D3"
                ctx.lineWidth = 2.5
                ctx.stroke()
                ctx.fillStyle = "#19213A"
                ctx.fillText(pt.consensus.toFixed(2), x(pt.round) - 13, y(pt.consensus) - 12)
            }
        }
    }
}
