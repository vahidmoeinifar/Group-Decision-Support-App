import QtQuick
import QtQuick.Shapes
import GroupDecision

// Three lines meeting in one point: several opinions, one decision.
Item {
    id: root

    property color lineColor: "#FFFFFF"
    property color dotColor: Theme.saffron

    implicitWidth: 30
    implicitHeight: 22

    Shape {
        anchors.fill: parent
        layer.enabled: true   // smooth (antialiased) curves
        layer.samples: 4

        ShapePath {
            strokeColor: root.lineColor
            strokeWidth: 2.2
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            startX: 2; startY: 3
            PathQuad { x: 20; y: 11; controlX: 12; controlY: 3 }
        }
        ShapePath {
            strokeColor: root.lineColor
            strokeWidth: 2.2
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            startX: 2; startY: 11
            PathLine { x: 20; y: 11 }
        }
        ShapePath {
            strokeColor: root.lineColor
            strokeWidth: 2.2
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            startX: 2; startY: 19
            PathQuad { x: 20; y: 11; controlX: 12; controlY: 19 }
        }
    }

    Rectangle {
        width: 9; height: 9; radius: 4.5
        color: root.dotColor
        x: 21; y: 6.5
    }
}
