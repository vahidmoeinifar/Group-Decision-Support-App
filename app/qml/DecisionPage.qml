import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GroupDecision

// One decision: header, round status and three tabs (Results, Scores, Setup).
Item {
    id: page

    required property DecisionController d
    property int tab: 0

    readonly property var tabs: ["Results", "Scores", "Setup"]

    function count(n, one, many) { return n + " " + (n === 1 ? one : many) }

    // Back to Results when another decision is opened
    Connections {
        target: page.d
        property int lastId: -1
        function onChanged() {
            if (page.d.decisionId !== lastId) {
                lastId = page.d.decisionId
                page.tab = 0
                scroll.ScrollBar.vertical.position = 0
            }
        }
    }

    ScrollView {
        id: scroll
        anchors.fill: parent
        contentWidth: availableWidth
        contentHeight: content.implicitHeight + 88

        ColumnLayout {
            id: content
            x: 44
            y: 44
            width: scroll.availableWidth - 88
            spacing: 22

            // ---- Header ----
            RowLayout {
                Layout.fillWidth: true
                spacing: 20

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Text {
                        Layout.fillWidth: true
                        text: page.d.title
                        color: Theme.ink
                        font.pixelSize: 34
                        font.weight: Font.Bold
                        wrapMode: Text.WordWrap
                        lineHeight: 1.05
                    }
                    Text {
                        Layout.fillWidth: true
                        visible: page.d.description !== ""
                        text: page.d.description
                        color: Theme.ink2
                        font.pixelSize: 16
                        wrapMode: Text.WordWrap
                    }
                    Text {
                        Layout.fillWidth: true
                        text: (page.d.isOwner ? "Organized by you. " : "Organized by " + page.d.ownerName + ". ")
                              + page.count(page.d.options.length, "option", "options") + ", "
                              + page.count(page.d.criteria.length, "criterion", "criteria") + ", "
                              + page.count(page.d.experts.length, "expert", "experts")
                              + ". Created " + page.d.createdText + "."
                        color: Theme.muted
                        wrapMode: Text.WordWrap
                    }
                }

                AppButton {
                    Layout.alignment: Qt.AlignTop
                    visible: page.d.isOwner
                    kind: "danger"
                    text: "Delete"
                    onClicked: deleteDialog.open()
                }
            }

            RoundBar {
                Layout.fillWidth: true
                d: page.d
                onReviewScores: page.tab = 1
            }

            // ---- Tabs ----
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                RowLayout {
                    spacing: 28
                    Repeater {
                        model: page.tabs
                        Item {
                            id: tabItem
                            required property int index
                            required property string modelData
                            implicitWidth: tabLabel.implicitWidth
                            implicitHeight: 40

                            Text {
                                id: tabLabel
                                anchors.top: parent.top
                                text: tabItem.modelData
                                font.pixelSize: 15
                                font.weight: page.tab === tabItem.index ? Font.DemiBold : Font.Medium
                                color: page.tab === tabItem.index ? Theme.ink : Theme.muted
                            }
                            Rectangle {
                                anchors.bottom: parent.bottom
                                width: parent.width
                                height: 3
                                radius: 1.5
                                color: Theme.ultra
                                visible: page.tab === tabItem.index
                            }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: page.tab = tabItem.index
                            }
                        }
                    }
                }
                Rectangle { Layout.fillWidth: true; height: 1; color: Theme.line }
            }

            // All three tabs stay alive, so unsaved scores survive a tab switch
            StackLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: [results, scores, setup][page.tab].implicitHeight
                currentIndex: page.tab

                ResultsTab { id: results; d: page.d; onGoToScores: page.tab = 1 }
                ScoresTab { id: scores; d: page.d; onShowResults: page.tab = 0 }
                SetupTab { id: setup; d: page.d }
            }
        }
    }

    ConfirmDialog {
        id: deleteDialog
        title: "Delete this decision?"
        message: "All options, criteria, experts and scores of \"" + page.d.title + "\" will be removed for everyone. This cannot be undone."
        confirmText: "Delete"
        destructive: true
        onAccepted: page.d.removeDecision()
    }
}
