import QtQuick
import QtQuick.Layouts
import GroupDecision

// A white panel with rounded corners. Children are stacked in a column.
Rectangle {
    id: card

    default property alias contentData: column.data
    property int padding: 24
    property alias spacing: column.spacing

    color: Theme.paper
    radius: Theme.radius
    border.color: Theme.line
    implicitHeight: column.implicitHeight + padding * 2
    implicitWidth: column.implicitWidth + padding * 2

    ColumnLayout {
        id: column
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: card.padding
        }
        spacing: 12
    }
}
