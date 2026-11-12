import QtQuick
import QtQuick.Layouts
import GroupDecision

// Signed-in layout: sidebar on the left, the open decision (or the new-decision form) on the right.
Item {
    id: workspace

    required property AppController app
    property bool creating: false

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Sidebar {
            Layout.fillHeight: true
            Layout.preferredWidth: 280
            app: workspace.app
            creating: workspace.creating
            onCreateRequested: workspace.creating = true
            onDecisionSelected: id => {
                workspace.creating = false
                workspace.app.current.open(id)
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // Created fresh each time, so the form starts empty
            Loader {
                anchors.fill: parent
                active: workspace.creating
                sourceComponent: NewDecisionPage {
                    controller: workspace.app.current
                    session: workspace.app.session
                    onFinished: workspace.creating = false
                }
            }

            DecisionPage {
                anchors.fill: parent
                visible: !workspace.creating && workspace.app.current.loaded
                d: workspace.app.current
            }

            EmptyState {
                anchors.centerIn: parent
                width: Math.min(460, parent.width - 80)
                visible: !workspace.creating && !workspace.app.current.loaded
                title: workspace.app.decisions.count === 0 ? "No decisions yet" : "Choose a decision"
                text: workspace.app.decisions.count === 0
                      ? "Create one with the New decision button, or ask an organizer to invite your email."
                      : "Pick a decision on the left to see its results and scores."
            }
        }
    }
}
