import QtQuick
import QtQuick.Layouts
import GroupDecision

// Slope chart: the rank of every option under each method, joined by lines.
// Crossing lines show where the choice of method changes the result.
Rectangle {
    id: chart

    property var comparison: ({})
    property var options: []   // [{id, name, index}]

    readonly property var methods: comparison.ok ? comparison.methods : []
    readonly property int rowCount: options.length
    readonly property real rowHeight: 58
    readonly property real labelSpace: Math.min(190, plot.width * 0.24)

    // Winner(s) under each method, and whether they all agree
    readonly property var winnerNames: methods.map(m => m.ranking.filter(r => r.rank === 1).map(r => r.option).join(" and "))
    readonly property bool sameWinner: winnerNames.length > 0 && winnerNames.every(w => w === winnerNames[0])

    function colorOf(optionId) {
        for (let i = 0; i < options.length; ++i)
            if (options[i].id === optionId) return Theme.optionColor(i)
        return Theme.muted
    }
    // Position (0 = top) of an option in one method's ranking
    function slot(methodIndex, optionId) {
        const r = methods[methodIndex].ranking
        for (let i = 0; i < r.length; ++i)
            if (r[i].optionId === optionId) return i
        return 0
    }
    function rankOf(methodIndex, optionId) {
        return methods[methodIndex].ranking[slot(methodIndex, optionId)].rank
    }
    function colX(k) { return labelSpace + k * (plot.width - 2 * labelSpace) / Math.max(1, methods.length - 1) }
    function rowY(i) { return 54 + i * rowHeight }

    implicitHeight: layout.implicitHeight + 64
    radius: 18
    color: Theme.ink

    ColumnLayout {
        id: layout
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 32 }
        spacing: 8

        Text {
            text: "Does the method change the result?"
            color: "#FFFFFF"
            font.pixelSize: 24
            font.weight: Font.Bold
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: chart.sameWinner ? "#B5BDD0" : Theme.saffron
            text: {
                if (chart.methods.length === 0) return ""
                if (chart.sameWinner) return chart.winnerNames[0] + " comes first with every method."
                return "The winner depends on the method: " + chart.methods.map(
                           (m, k) => Theme.methodLabel(m.method) + " picks " + chart.winnerNames[k]).join(", ") + "."
            }
        }

        Item {
            id: plot
            Layout.fillWidth: true
            Layout.topMargin: 10
            implicitHeight: chart.rowY(chart.rowCount - 1) + 34

            // Column titles
            Repeater {
                model: chart.methods
                Text {
                    required property int index
                    required property var modelData
                    x: chart.colX(index) - width / 2
                    y: 0
                    text: Theme.methodLabel(modelData.method)
                    color: "#B5BDD0"
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                }
            }

            // Lines between the columns
            Canvas {
                id: canvas
                anchors.fill: parent
                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    if (chart.methods.length === 0) return
                    ctx.strokeStyle = "rgba(255,255,255,0.10)"
                    ctx.lineWidth = 1
                    for (let k = 0; k < chart.methods.length; ++k) {
                        ctx.beginPath()
                        ctx.moveTo(chart.colX(k), 30)
                        ctx.lineTo(chart.colX(k), height)
                        ctx.stroke()
                    }
                    ctx.lineWidth = 3.5
                    for (let i = 0; i < chart.options.length; ++i) {
                        const id = chart.options[i].id
                        ctx.strokeStyle = Theme.optionColor(i)
                        ctx.beginPath()
                        for (let k = 0; k < chart.methods.length; ++k) {
                            const x = chart.colX(k), y = chart.rowY(chart.slot(k, id))
                            if (k === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
                        }
                        ctx.stroke()
                    }
                }
                onWidthChanged: requestPaint()
                Connections {
                    target: chart
                    function onComparisonChanged() { canvas.requestPaint() }
                }
            }

            // Rank dots
            Repeater {
                model: chart.methods.length * chart.options.length
                Rectangle {
                    required property int index
                    readonly property int k: Math.floor(index / chart.options.length)
                    readonly property var option: chart.options[index % chart.options.length]
                    readonly property int rank: chart.methods.length > 0 ? chart.rankOf(k, option.id) : 0

                    width: 30; height: 30; radius: 15
                    x: chart.colX(k) - 15
                    y: chart.rowY(chart.slot(k, option.id)) - 15
                    color: chart.colorOf(option.id)
                    border.width: rank === 1 ? 3 : 0
                    border.color: Theme.saffron

                    Text {
                        anchors.centerIn: parent
                        text: parent.rank
                        color: "#FFFFFF"
                        font.weight: Font.Bold
                    }
                }
            }

            // Option names on both sides
            Repeater {
                model: chart.methods.length > 0 ? chart.options : []
                Item {
                    id: labels
                    required property var modelData
                    readonly property var first: chart.methods[0]
                    readonly property int lastK: chart.methods.length - 1

                    Text {
                        x: chart.colX(0) - 26 - width
                        y: chart.rowY(chart.slot(0, labels.modelData.id)) - height / 2
                        text: labels.modelData.name
                        color: "#FFFFFF"
                        font.pixelSize: 16
                        font.weight: Font.DemiBold
                    }
                    Text {
                        x: chart.colX(labels.lastK) + 26
                        y: chart.rowY(chart.slot(labels.lastK, labels.modelData.id)) - height / 2
                        text: labels.modelData.name
                        color: "#FFFFFF"
                        font.pixelSize: 16
                        font.weight: Font.DemiBold
                    }
                }
            }
        }
    }
}
