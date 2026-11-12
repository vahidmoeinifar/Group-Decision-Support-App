import QtQuick
import QtQuick.Layouts
import GroupDecision

// Short messages in the bottom-right corner that disappear after a few seconds.
Item {
    id: root

    function show(message, kind) {
        messages.append({ message: message, kind: kind || "info" })
    }

    ListModel { id: messages }

    ColumnLayout {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 24
        spacing: 10

        Repeater {
            model: messages

            Rectangle {
                id: toast
                required property int index
                required property string message
                required property string kind

                Layout.alignment: Qt.AlignRight
                implicitWidth: Math.min(420, text.implicitWidth + 44)
                implicitHeight: text.implicitHeight + 26
                radius: 12
                color: "#232C4A"
                border.color: "#46507A"   // visible on dark panels too
                opacity: 0

                Rectangle {
                    width: 4
                    radius: 2
                    anchors { left: parent.left; top: parent.top; bottom: parent.bottom; margins: 8 }
                    color: toast.kind === "error" ? "#F2788C" : toast.kind === "success" ? "#4CC38A" : Theme.saffron
                }

                Text {
                    id: text
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 24
                    width: Math.min(implicitWidth, 376)
                    text: toast.message
                    color: "#FFFFFF"
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.bodySize
                }

                NumberAnimation on opacity { to: 1; duration: 160 }

                Timer {
                    interval: toast.kind === "error" ? 6000 : 4000
                    running: true
                    onTriggered: messages.remove(toast.index)
                }
            }
        }
    }
}
