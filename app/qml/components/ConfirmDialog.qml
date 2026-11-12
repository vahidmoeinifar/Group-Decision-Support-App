import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GroupDecision

// "Are you sure?" dialog. Connect to accepted().
Dialog {
    id: dialog

    property string message: ""
    property string confirmText: "Confirm"
    property bool destructive: false

    modal: true
    anchors.centerIn: Overlay.overlay
    width: Math.min(460, parent ? parent.width - 48 : 460)
    padding: 26

    Overlay.modal: Rectangle { color: "#8019213A" }

    // The title is shown inside the content below, so hide the built-in header
    header: Item { implicitHeight: 0 }

    background: Rectangle {
        radius: 16
        color: Theme.paper
        border.color: Theme.line
    }

    // Title, message and buttons in one column (no separate header/footer,
    // which keeps the height calculation simple)
    contentItem: ColumnLayout {
        spacing: 14

        Text {
            Layout.fillWidth: true
            text: dialog.title
            color: Theme.ink
            font.pixelSize: 19
            font.weight: Font.Bold
            wrapMode: Text.WordWrap
        }
        Text {
            Layout.fillWidth: true
            text: dialog.message
            color: Theme.ink2
            wrapMode: Text.WordWrap
            lineHeight: 1.25
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 8
            spacing: 10
            Item { Layout.fillWidth: true }
            AppButton { text: "Cancel"; onClicked: dialog.reject() }
            AppButton {
                text: dialog.confirmText
                kind: dialog.destructive ? "danger" : "primary"
                onClicked: dialog.accept()
            }
        }
    }
}
