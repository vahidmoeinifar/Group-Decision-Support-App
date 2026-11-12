import QtQuick
import QtQuick.Controls
import GroupDecision

// Text input with the app's look. Set `invalid` to show a red border,
// `highlighted` for an orange one (used for scores far from the group).
TextField {
    id: control

    property bool invalid: false
    property bool highlighted: false
    property bool edited: false     // changed but not saved yet

    font.pixelSize: Theme.bodySize
    color: Theme.ink
    placeholderTextColor: Theme.muted
    selectByMouse: true
    leftPadding: 12
    rightPadding: 12
    topPadding: 9
    bottomPadding: 9

    background: Rectangle {
        implicitWidth: 120
        implicitHeight: 40
        radius: 9
        color: control.readOnly ? Theme.lineSoft : control.edited ? "#F3F5FE" : "#FFFFFF"
        border.width: control.activeFocus || control.invalid || control.highlighted || control.edited ? 2 : 1
        border.color: control.invalid ? Theme.red
                    : control.activeFocus || control.edited ? Theme.ultra
                    : control.highlighted ? Theme.saffron : Theme.line
    }
}
