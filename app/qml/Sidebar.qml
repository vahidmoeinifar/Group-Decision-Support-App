import QtQuick
import QtQuick.Layouts
import GroupDecision

// Left column: logo, new-decision button, list of decisions, live status and account.
Rectangle {
    id: sidebar

    required property AppController app
    property bool creating: false

    signal createRequested()
    signal decisionSelected(int id)

    color: Theme.ink

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 14

        RowLayout {
            Layout.topMargin: 8
            Layout.leftMargin: 6
            spacing: 12
            Logo { }
            Text { text: "Group Decision"; color: "#FFFFFF"; font.pixelSize: 18; font.weight: Font.Bold }
        }

        AppButton {
            Layout.fillWidth: true
            Layout.topMargin: 8
            kind: "dark"
            text: "+   New decision"
            onClicked: sidebar.createRequested()
        }

        Text {
            Layout.topMargin: 8
            Layout.leftMargin: 6
            text: "Your decisions"
            color: Theme.sideMuted
            font.pixelSize: 13
            font.weight: Font.DemiBold
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 6
            model: sidebar.app.decisions

            delegate: Rectangle {
                id: item
                required property int decisionId
                required property string title
                required property string roleText

                readonly property bool active: !sidebar.creating && sidebar.app.current.decisionId === decisionId

                width: ListView.view.width
                implicitHeight: texts.implicitHeight + 22
                radius: 10
                color: active ? Theme.sideItem : mouse.containsMouse ? "#212A47" : "transparent"

                Rectangle {
                    visible: item.active
                    width: 4
                    height: parent.height - 16
                    anchors.verticalCenter: parent.verticalCenter
                    x: -18 + 2
                    radius: 2
                    color: Theme.saffron
                }

                ColumnLayout {
                    id: texts
                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 12 }
                    spacing: 3
                    Text {
                        Layout.fillWidth: true
                        text: item.title
                        color: "#FFFFFF"
                        font.weight: Font.DemiBold
                        wrapMode: Text.WordWrap
                    }
                    Text {
                        text: item.roleText
                        color: Theme.sideMuted
                        font.pixelSize: 13
                    }
                }

                MouseArea {
                    id: mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: sidebar.decisionSelected(item.decisionId)
                }
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#2C3558" }

        RowLayout {
            spacing: 8
            Rectangle {
                implicitWidth: 9; implicitHeight: 9; radius: 4.5
                color: sidebar.app.live ? "#4CC38A" : Theme.sideMuted
                SequentialAnimation on opacity {
                    running: sidebar.app.live
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.35; duration: 900 }
                    NumberAnimation { to: 1; duration: 900 }
                }
            }
            Text {
                text: sidebar.app.live ? "Live updates on" : "Live updates off"
                color: Theme.sideText
                font.pixelSize: 13
            }
        }

        RowLayout {
            spacing: 12
            Avatar { initials: sidebar.app.session.initials }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1
                Text {
                    Layout.fillWidth: true
                    text: sidebar.app.session.displayName
                    color: "#FFFFFF"
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }
                Text {
                    Layout.fillWidth: true
                    text: sidebar.app.session.email
                    color: Theme.sideMuted
                    font.pixelSize: 13
                    elide: Text.ElideRight
                }
            }
        }

        AppButton {
            Layout.fillWidth: true
            kind: "dark"
            text: "Sign out"
            onClicked: sidebar.app.session.signOut()
        }

        Text {
            Layout.fillWidth: true
            color: Theme.sideMuted
            font.pixelSize: 11
            elide: Text.ElideRight
            text: "By Vahid Moeinifar"
        }
    }
}
