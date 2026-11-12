import QtQuick
import QtQuick.Controls
import GroupDecision

// One button type for the whole app.
// kind: "primary" (filled), "secondary" (outlined), "quiet" (light), "danger", "link", "dark" (on the sidebar)
Button {
    id: control

    property string kind: "secondary"

    readonly property bool filled: kind === "primary"
    readonly property color textColor: {
        if (!enabled) return kind === "primary" ? "#FFFFFF" : Theme.muted
        switch (kind) {
        case "primary": return "#FFFFFF"
        case "danger": return Theme.red
        case "link": return Theme.ultra
        case "dark": return "#FFFFFF"
        default: return Theme.ink
        }
    }

    font.pixelSize: Theme.bodySize
    font.weight: Font.DemiBold
    padding: kind === "link" ? 2 : 10
    leftPadding: kind === "link" ? 2 : 18
    rightPadding: kind === "link" ? 2 : 18
    hoverEnabled: true

    contentItem: Text {
        text: control.text
        font.pixelSize: control.font.pixelSize
        font.weight: control.font.weight
        color: control.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        font.underline: control.kind === "link" && control.hovered
    }

    background: Rectangle {
        implicitHeight: control.kind === "link" ? 20 : 40
        radius: 10
        visible: control.kind !== "link"
        color: {
            switch (control.kind) {
            case "primary":
                return !control.enabled ? "#A3ACE9" : control.down ? Theme.ultraDark
                     : control.hovered ? "#3E51E0" : Theme.ultra
            case "quiet":
                return control.hovered ? "#F2E2C2" : Theme.saffronSoft
            case "dark":
                return control.hovered ? "#323C60" : Theme.sideItem
            default:
                return control.hovered && control.enabled ? "#FFFFFF" : Theme.paper
            }
        }
        border.width: control.kind === "secondary" || control.kind === "danger" ? 1 : 0
        border.color: control.kind === "danger" && control.hovered ? Theme.red : Theme.line

        Behavior on color { ColorAnimation { duration: 120 } }
    }
}
