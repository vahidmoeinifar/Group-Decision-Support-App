import QtQuick
import QtQuick.Layouts
import GroupDecision

// Group ranking, agreement, method comparison, agreement history and sensitivity.
ColumnLayout {
    id: tab

    required property DecisionController d
    property string method: "Topsis"
    signal goToScores()

    readonly property var methods: ["Topsis", "WeightedSum", "Borda"]
    readonly property var methodHelp: ({
        Topsis: "Ranks options by how close they are to an ideal option and how far they are from the worst one.",
        WeightedSum: "Scales every criterion to 0–1 and adds them up using the weights. Simple and easy to explain.",
        Borda: "Each expert ranks the options on their own; the rankings are then combined by points, like a vote."
    })

    // Using d.revision in each binding makes it recompute whenever the decision changes.
    // (A bare "d.revision;" statement is not enough: the QML compiler may drop it.)
    readonly property var result: d.revision >= 0 ? d.result(method) : ({})
    readonly property var comparison: d.revision >= 0 ? d.compare() : ({})
    readonly property var history: d.revision >= 0 ? d.history() : []
    readonly property var sensitivity: d.revision >= 0 ? d.sensitivity(method) : ({})
    readonly property bool wide: width > 860

    spacing: 20

    Card {
        Layout.fillWidth: true
        visible: !tab.result.ok
        EmptyState {
            Layout.fillWidth: true
            title: "No results yet"
            text: tab.result.error || ""
        }
        AppButton {
            visible: tab.d.myExpertId >= 0
            kind: "primary"
            text: "Enter my scores"
            onClicked: tab.goToScores()
        }
    }

    GridLayout {
        Layout.fillWidth: true
        visible: tab.result.ok
        columns: tab.wide ? 2 : 1
        columnSpacing: 20
        rowSpacing: 20

        Card {
            Layout.fillWidth: true
            Layout.preferredWidth: 1   // take the space left by the agreement panel
            Layout.alignment: Qt.AlignTop
            spacing: 14

            Segmented {
                Layout.fillWidth: !tab.wide
                model: tab.methods.map(m => Theme.methodLabel(m))
                currentIndex: tab.methods.indexOf(tab.method)
                onActivated: index => tab.method = tab.methods[index]
            }
            Text {
                Layout.fillWidth: true
                text: tab.methodHelp[tab.method]
                color: Theme.ink2
                wrapMode: Text.WordWrap
            }
            RankingList {
                Layout.fillWidth: true
                Layout.topMargin: 6
                ranking: tab.result.ok ? tab.result.ranking : []
            }
        }

        AgreementPanel {
            Layout.fillWidth: !tab.wide
            Layout.preferredWidth: tab.wide ? 330 : -1
            Layout.alignment: Qt.AlignTop
            result: tab.result
            target: tab.d.consensusTarget
        }
    }

    MethodComparison {
        Layout.fillWidth: true
        visible: tab.comparison.ok
        comparison: tab.comparison
        options: tab.d.options
    }

    GridLayout {
        Layout.fillWidth: true
        visible: tab.result.ok
        columns: tab.wide ? 2 : 1
        columnSpacing: 20
        rowSpacing: 20

        AgreementHistory {
            Layout.fillWidth: true
            Layout.preferredWidth: 1   // equal columns
            Layout.alignment: Qt.AlignTop
            rounds: tab.history
            target: tab.d.consensusTarget
        }

        SensitivityPanel {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            Layout.alignment: Qt.AlignTop
            info: tab.sensitivity
            options: tab.d.options
        }
    }
}
