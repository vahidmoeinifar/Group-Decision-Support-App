import QtQuick
import QtQuick.Layouts
import GroupDecision

// The score grid. Experts edit their own scores in the current round.
// The organizer can look at every expert's scores (read only).
// From round 2 on, cells far from the group are highlighted with a suggested value.
ColumnLayout {
    id: tab

    required property DecisionController d
    signal showResults()

    property int expertId: -1
    // "optionId:criterionId" -> text typed but not saved yet.
    // Always replaced with a new object (never changed in place), so bindings notice the change.
    property var drafts: ({})

    readonly property var expert: d.experts.find(e => e.id === expertId) || null
    readonly property bool editable: expert !== null && expert.isMe
    readonly property var saved: d.revision >= 0 && expertId >= 0 ? d.scores(expertId) : ({ ok: false })
    readonly property var feedback: d.revision >= 0 && expert && expert.isComplete ? d.feedback(expertId) : ({ ok: false })
    readonly property int total: d.options.length * d.criteria.length
    readonly property int filled: countFilled()
    readonly property bool dirty: Object.keys(drafts).length > 0
    readonly property bool hasInvalid: Object.keys(drafts).some(k => !isNumber(drafts[k]))

    function key(optionId, criterionId) { return optionId + ":" + criterionId }
    function isNumber(text) { return String(text).trim() !== "" && isFinite(Number(text)) }

    function savedText(k) {
        return saved.ok && saved.cells[k] !== undefined ? Theme.num(saved.cells[k], 4) : ""
    }
    function cellText(k) {
        return drafts[k] !== undefined ? drafts[k] : savedText(k)
    }
    function setDraft(k, text) {
        const copy = Object.assign({}, drafts)
        if (text === savedText(k)) delete copy[k]; else copy[k] = text
        drafts = copy
    }
    function countFilled() {
        let n = 0
        for (const o of d.options)
            for (const c of d.criteria)
                if (isNumber(cellText(key(o.id, c.id)))) n++
        return n
    }
    function cellFeedback(k) {
        return feedback.ok ? feedback.cells[k] : undefined
    }
    function moveHalfway() {
        const copy = Object.assign({}, drafts)
        for (const k in feedback.cells)
            if (feedback.cells[k].flagged) copy[k] = Theme.num(feedback.cells[k].suggested, 2)
        drafts = copy
    }
    function save() {
        const cells = {}
        for (const k in drafts)
            if (isNumber(drafts[k])) cells[k] = Number(drafts[k])
        if (d.saveScores(expertId, cells)) {
            drafts = {}
        }
    }
    function pickExpert() {
        if (d.myExpertId >= 0) { expertId = d.myExpertId; return }
        expertId = d.experts.length > 0 ? d.experts[0].id : -1
    }

    Component.onCompleted: pickExpert()

    Connections {
        target: tab.d
        property int lastId: -1
        function onChanged() {
            if (tab.d.decisionId !== lastId) {   // another decision was opened
                lastId = tab.d.decisionId
                tab.drafts = {}
                tab.pickExpert()
            }
        }
    }

    spacing: 18

    // ---- Organizer: choose whose scores to look at ----
    Flow {
        Layout.fillWidth: true
        visible: tab.d.isOwner
        spacing: 10

        Repeater {
            model: tab.d.experts

            Rectangle {
                id: chip
                required property var modelData
                readonly property bool selected: modelData.id === tab.expertId

                implicitWidth: chipRow.implicitWidth + 30
                implicitHeight: 42
                radius: 21
                color: selected ? "#FFFFFF" : Theme.paper
                border.width: selected ? 2 : 1
                border.color: selected ? Theme.ultra : Theme.line

                RowLayout {
                    id: chipRow
                    anchors.centerIn: parent
                    spacing: 8
                    Rectangle {
                        implicitWidth: 8; implicitHeight: 8; radius: 4   // inside a layout: implicit size, not width
                        color: chip.modelData.isComplete ? Theme.green : chip.modelData.filled > 0 ? Theme.saffron : "#C3CAD8"
                    }
                    Text { text: chip.modelData.name + (chip.modelData.isMe ? " (you)" : ""); color: Theme.ink; font.weight: Font.DemiBold }
                    Text {
                        color: Theme.muted
                        font.pixelSize: 13
                        text: !chip.modelData.hasAccount ? "No account yet"
                              : chip.modelData.isComplete ? "Complete"
                              : chip.modelData.filled + " of " + chip.modelData.total
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        tab.expertId = chip.modelData.id
                        tab.drafts = {}
                    }
                }
            }
        }
    }

    EmptyState {
        Layout.fillWidth: true
        visible: tab.expert === null
        title: "You are not an expert in this decision"
        text: "Only invited experts enter scores."
    }

    // ---- Heading and progress ----
    RowLayout {
        Layout.fillWidth: true
        visible: tab.expert !== null
        spacing: 24

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Text {
                text: tab.expert === null ? ""
                      : (tab.editable ? "Your scores, round " : "Scores from " + tab.expert.name + ", round ") + tab.d.currentRound
                color: Theme.ink
                font.pixelSize: 22
                font.weight: Font.Bold
            }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: Theme.ink2
                text: {
                    if (tab.expert === null) return ""
                    if (tab.editable) return "Enter a value for every option on every criterion, in the unit the criterion is named in."
                    if (!tab.expert.hasAccount) return tab.expert.name + " was invited as " + tab.expert.email + " but has not created an account yet."
                    return "Only " + tab.expert.name + " can change these scores."
                }
            }
        }

        ColumnLayout {
            Layout.preferredWidth: 200
            spacing: 6
            Text {
                Layout.alignment: Qt.AlignRight
                text: "<b>" + tab.filled + "</b> of " + tab.total + " filled"
                textFormat: Text.StyledText
                color: Theme.ink2
            }
            Rectangle {
                Layout.fillWidth: true
                height: 6
                radius: 3
                color: Theme.lineSoft
                Rectangle {
                    height: parent.height
                    radius: 3
                    width: tab.total > 0 ? parent.width * tab.filled / tab.total : 0
                    color: tab.filled === tab.total ? Theme.green : Theme.ultra
                }
            }
        }
    }

    // ---- Feedback from round 2 on ----
    Rectangle {
        Layout.fillWidth: true
        visible: tab.feedback.ok
        implicitHeight: feedbackRow.implicitHeight + 36
        radius: Theme.radius
        color: tab.feedback.ok && tab.feedback.flaggedCount === 0 ? Theme.greenSoft : Theme.saffronSoft
        border.color: tab.feedback.ok && tab.feedback.flaggedCount === 0 ? "#BFE3D0" : "#F0D6A8"

        RowLayout {
            id: feedbackRow
            anchors.fill: parent
            anchors.margins: 18
            anchors.leftMargin: 22
            spacing: 20

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    color: Theme.ink
                    font.weight: Font.DemiBold
                    text: !tab.feedback.ok ? ""
                          : tab.feedback.flaggedCount === 0
                            ? (tab.editable ? "Your scores are close to the group." : tab.feedback.expert + "'s scores are close to the group.")
                            : tab.feedback.flaggedCount + (tab.editable ? " of your scores are far from the group." : " scores are far from the group.")
                }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    color: Theme.ink2
                    lineHeight: 1.2
                    text: !tab.feedback.ok ? ""
                          : (tab.editable ? "Your own" : tab.feedback.expert + "'s") + " ranking agrees "
                            + Theme.fixed(tab.feedback.agreement) + " with the group ranking."
                            + (tab.feedback.flaggedCount === 0 ? ""
                               : tab.editable
                               ? " Highlighted cells show the group average and a value halfway to it. Moving closer raises agreement, but only change what you honestly can."
                               : " Highlighted cells show the group average.")
                }
            }

            AppButton {
                visible: tab.editable && tab.feedback.ok && tab.feedback.flaggedCount > 0
                text: "Move highlighted scores halfway"
                onClicked: tab.moveHalfway()
            }
        }
    }

    // ---- The grid ----
    Rectangle {
        id: grid
        Layout.fillWidth: true
        visible: tab.expert !== null
        implicitHeight: gridColumn.implicitHeight
        radius: Theme.radius
        color: Theme.paper
        border.color: Theme.line

        readonly property int firstColumn: width < 700 ? 130 : 170

        ColumnLayout {
            id: gridColumn
            anchors { left: parent.left; right: parent.right; top: parent.top }
            spacing: 0

            // Header row
            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 16
                Layout.bottomMargin: 12
                spacing: 18

                Text {
                    Layout.preferredWidth: grid.firstColumn - 16
                    Layout.alignment: Qt.AlignBottom
                    text: "Option"
                    color: Theme.muted
                    font.weight: Font.DemiBold
                }
                Repeater {
                    model: tab.d.criteria
                    ColumnLayout {
                        id: head
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        spacing: 4
                        Text {
                            Layout.fillWidth: true
                            text: head.modelData.name
                            color: Theme.ink
                            font.weight: Font.DemiBold
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: true
                            text: head.modelData.isCost ? "Lower is better" : "Higher is better"
                            color: head.modelData.isCost ? "#B5741A" : Theme.green
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: true
                            text: "Weight " + Theme.pct(head.modelData.share)
                            color: Theme.muted
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                    }
                }
            }

            // One row per option
            Repeater {
                model: tab.d.options

                Rectangle {
                    id: optionRow
                    required property var modelData
                    required property int index

                    Layout.fillWidth: true
                    implicitHeight: cells.implicitHeight + 26
                    color: "transparent"

                    Rectangle { width: parent.width; height: 1; color: Theme.lineSoft }

                    RowLayout {
                        id: cells
                        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; leftMargin: 16; rightMargin: 16 }
                        spacing: 18

                        RowLayout {
                            // Layouts fill by default; this one must keep a fixed width
                            Layout.fillWidth: false
                            Layout.preferredWidth: grid.firstColumn - 16
                            Layout.alignment: Qt.AlignTop
                            Layout.topMargin: 10
                            spacing: 10
                            Rectangle { implicitWidth: 10; implicitHeight: 10; radius: 5; color: Theme.optionColor(optionRow.index) }
                            Text {
                                Layout.fillWidth: true
                                text: optionRow.modelData.name
                                color: Theme.ink
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }
                        }

                        Repeater {
                            model: tab.d.criteria

                            ColumnLayout {
                                id: cell
                                required property var modelData
                                readonly property string k: tab.key(optionRow.modelData.id, modelData.id)
                                readonly property var fb: tab.cellFeedback(k)
                                readonly property bool flagged: fb !== undefined && fb.flagged

                                Layout.fillWidth: true
                                Layout.preferredWidth: 1
                                Layout.alignment: Qt.AlignTop
                                spacing: 4

                                AppField {
                                    Layout.fillWidth: true
                                    horizontalAlignment: TextInput.AlignRight
                                    readOnly: !tab.editable
                                    placeholderText: tab.editable ? "" : "–"
                                    text: tab.cellText(cell.k)
                                    invalid: tab.drafts[cell.k] !== undefined && !tab.isNumber(tab.drafts[cell.k])
                                    edited: tab.drafts[cell.k] !== undefined
                                    highlighted: cell.flagged
                                    onTextEdited: {
                                        tab.setDraft(cell.k, text)
                                        // Typing replaced the binding; put it back so "Use x" buttons can still update the field
                                        text = Qt.binding(() => tab.cellText(cell.k))
                                    }
                                    Accessible.name: optionRow.modelData.name + ", " + cell.modelData.name
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    visible: cell.flagged
                                    Text {
                                        text: cell.flagged ? "Group " + Theme.num(cell.fb.group, 2) : ""
                                        color: "#B5741A"
                                        font.pixelSize: 12
                                    }
                                    Item { Layout.fillWidth: true }
                                    AppButton {
                                        visible: tab.editable
                                        kind: "link"
                                        font.pixelSize: 12
                                        text: cell.flagged ? "Use " + Theme.num(cell.fb.suggested, 2) : ""
                                        onClicked: tab.setDraft(cell.k, Theme.num(cell.fb.suggested, 2))
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // ---- Actions ----
    RowLayout {
        visible: tab.editable
        spacing: 16
        AppButton {
            kind: "primary"
            text: "Save scores"
            enabled: tab.dirty && !tab.hasInvalid
            onClicked: tab.save()
        }
        AppButton { kind: "link"; text: "See results"; onClicked: tab.showResults() }
        Text {
            visible: tab.hasInvalid
            text: "Some values are not numbers."
            color: Theme.red
        }
        Text {
            visible: tab.dirty && !tab.hasInvalid
            text: "You have unsaved changes."
            color: Theme.muted
        }
    }
}
