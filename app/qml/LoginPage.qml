import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GroupDecision

// Sign in or create an account. Demo accounts sign in with one click.
Item {
    id: page

    required property Session session
    property bool registering: false

    readonly property var demoAccounts: [
        { email: "organizer@demo.local", name: "Anna Nowak", role: "Organizer: runs the decision and its rounds" },
        { email: "head@demo.local", name: "Piotr Wiśniewski", role: "Expert: head of lab" },
        { email: "engineer@demo.local", name: "Kasia Zielińska", role: "Expert: engineer" },
        { email: "finance@demo.local", name: "Marek Lewandowski", role: "Expert: finance officer" }
    ]

    function submit() {
        if (registering)
            session.registerAccount(nameField.text, emailField.text, passwordField.text)
        else
            session.signIn(emailField.text, passwordField.text)
    }

    function initialsOf(name) {
        return name.split(" ").map(p => p[0]).join("").slice(0, 2)
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ---- Left: brand panel ----
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.48
            color: Theme.ink
            visible: page.width > 980

            RowLayout {
                anchors { left: parent.left; top: parent.top; margins: 30 }
                spacing: 12
                Logo { }

                ColumnLayout {
                    anchors { left: parent.left + 15; top: parent.top; margins: 12 }
                    spacing: 12
                    Text { text: "Group Decision"; color: "#FFFFFF"; font.pixelSize: 21; font.weight: Font.Bold }
                    Text { text: "By Vahid Moeinifar"; color: Theme.sideMuted; font.pixelSize: 11; font.weight: Font.Bold }

                }

            }

            // Five opinions flowing into one decision
            Canvas {
                id: lines
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -60
                width: Math.min(parent.width - 120, 460)
                height: 270

                property real progress: 0
                NumberAnimation on progress { from: 0; to: 1; duration: 1400; easing.type: Easing.OutCubic }
                onProgressChanged: requestPaint()

                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    const endX = width - 40, endY = height / 2
                    ctx.lineWidth = 2.5
                    ctx.strokeStyle = "#7E8BEA"
                    for (let k = 0; k < 5; ++k) {
                        const y0 = 30 + k * (height - 60) / 4
                        ctx.beginPath()
                        ctx.moveTo(10, y0)
                        const x1 = 10 + (endX - 10) * progress
                        ctx.bezierCurveTo(10 + (endX - 10) * 0.45 * progress, y0,
                                          10 + (endX - 10) * 0.55 * progress, y0 + (endY - y0) * progress,
                                          x1, y0 + (endY - y0) * progress)
                        ctx.stroke()
                        ctx.fillStyle = "#E7EAF7"
                        ctx.beginPath()
                        ctx.arc(10, y0, 7, 0, Math.PI * 2)
                        ctx.fill()
                    }
                    ctx.fillStyle = "#E8A33B"
                    ctx.beginPath()
                    ctx.arc(endX, endY, 18 * progress, 0, Math.PI * 2)
                    ctx.fill()
                }
            }

            ColumnLayout {
                anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: 56 }
                spacing: 14
                Text {
                    text: "Several experts,\none decision."
                    color: "#FFFFFF"
                    font.pixelSize: 46
                    font.weight: Font.Bold
                    lineHeight: 0.95
                }
                Text {
                    Layout.fillWidth: true
                    Layout.maximumWidth: 460
                    text: "Collect scores from every expert, see where the group agrees, and run rounds until the decision is one everyone can stand behind."
                    color: "#B5BDD0"
                    font.pixelSize: 16
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                }
            }
        }

        // ---- Right: the form ----
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollView {
                anchors.fill: parent
                contentWidth: availableWidth

                ColumnLayout {
                    width: Math.min(440, parent.width - 48)
                    x: (parent.width - width) / 2
                    spacing: 16

                    Item { Layout.preferredHeight: Math.max(40, (page.height - 760) / 2) }

                    Segmented {
                        Layout.fillWidth: true
                        model: ["Sign in", "Create account"]
                        currentIndex: page.registering ? 1 : 0
                        onActivated: index => { page.registering = index === 1; page.session.clearError() }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        visible: page.registering
                        Text { text: "Your name"; color: Theme.ink; font.weight: Font.DemiBold }
                        AppField { id: nameField; Layout.fillWidth: true; onAccepted: page.submit() }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Text { text: "Email"; color: Theme.ink; font.weight: Font.DemiBold }
                        AppField { id: emailField; Layout.fillWidth: true; onAccepted: page.submit() }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Text { text: "Password"; color: Theme.ink; font.weight: Font.DemiBold }
                        AppField {
                            id: passwordField
                            Layout.fillWidth: true
                            echoMode: TextInput.Password
                            placeholderText: page.registering ? "At least 8 characters" : ""
                            onAccepted: page.submit()
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        visible: page.session.error !== ""
                        text: page.session.error
                        color: Theme.red
                        wrapMode: Text.WordWrap
                    }

                    AppButton {
                        Layout.fillWidth: true
                        kind: "primary"
                        text: page.registering ? "Create account" : "Sign in"
                        onClicked: page.submit()
                    }

                    Text {
                        Layout.fillWidth: true
                        visible: page.registering
                        text: "If someone invited your email to a decision, it appears as soon as you create the account."
                        color: Theme.muted
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                    }

                    Rectangle { Layout.fillWidth: true; Layout.topMargin: 10; height: 1; color: Theme.line }

                    Text {
                        text: "Try it with a demo account"
                        color: Theme.ink2
                        font.weight: Font.DemiBold
                    }

                    Repeater {
                        model: page.demoAccounts

                        Rectangle {
                            id: account
                            required property var modelData
                            Layout.fillWidth: true
                            implicitHeight: 64
                            radius: 12
                            color: mouse.containsMouse ? "#FFFFFF" : Theme.paper
                            border.color: mouse.containsMouse ? Theme.ultra : Theme.line

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 14
                                anchors.rightMargin: 14
                                spacing: 12
                                Avatar {
                                    initials: page.initialsOf(account.modelData.name)
                                    fill: Theme.ultraSoft
                                    textColor: Theme.ultra
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Text { Layout.fillWidth: true; text: account.modelData.name; color: Theme.ink; font.weight: Font.DemiBold; font.pixelSize: 15 }
                                    Text { Layout.fillWidth: true; text: account.modelData.role; color: Theme.muted; font.pixelSize: 13 }
                                }
                            }

                            MouseArea {
                                id: mouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: page.session.signIn(account.modelData.email, page.session.demoPassword)
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: "Demo password for all accounts: " + page.session.demoPassword
                        color: Theme.muted
                        font.pixelSize: 13
                    }

                    Item { Layout.preferredHeight: 40 }
                }
            }
        }
    }
}
