import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GroupDecision

// Form to create a decision. The signed-in user becomes its organizer.
Item {
    id: page

    required property DecisionController controller
    required property Session session
    signal finished()

    property real target: 0.8
    property bool addMe: true

    ListModel {
        id: optionsModel
        ListElement { name: "" }
        ListElement { name: "" }
    }
    ListModel {
        id: criteriaModel
        ListElement { name: ""; weight: "1"; isCost: false }
    }
    ListModel {
        id: expertsModel
        ListElement { name: ""; email: ""; weight: "1" }
    }

    function totalWeight(model) {
        let sum = 0
        for (let i = 0; i < model.count; ++i) {
            const w = Number(model.get(i).weight)
            if (isFinite(w) && w > 0) sum += w
        }
        return sum
    }

    function submit() {
        const form = {
            title: titleField.text,
            description: descriptionField.text,
            target: page.target,
            addMe: page.addMe,
            options: [], criteria: [], experts: []
        }
        for (let i = 0; i < optionsModel.count; ++i)
            form.options.push(optionsModel.get(i).name)
        for (let i = 0; i < criteriaModel.count; ++i) {
            const c = criteriaModel.get(i)
            form.criteria.push({ name: c.name, weight: Number(c.weight), isCost: c.isCost })
        }
        for (let i = 0; i < expertsModel.count; ++i) {
            const e = expertsModel.get(i)
            if (e.name.trim() === "" && e.email.trim() === "") continue   // skip empty rows
            form.experts.push({ name: e.name, email: e.email, weight: Number(e.weight) })
        }
        if (page.controller.create(form) >= 0)
            page.finished()
    }

    // Small text button to remove a row
    component RemoveButton: AppButton {
        kind: "link"
        text: "Remove"
        font.pixelSize: 13
    }

    component SectionTitle: ColumnLayout {
        property string title: ""
        property string hint: ""
        Layout.fillWidth: true
        Layout.topMargin: 10
        spacing: 3
        Text { text: parent.title; color: Theme.ink; font.pixelSize: 19; font.weight: Font.Bold }
        Text { Layout.fillWidth: true; text: parent.hint; color: Theme.ink2; wrapMode: Text.WordWrap }
    }

    ScrollView {
        id: scroll
        anchors.fill: parent
        contentWidth: availableWidth
        contentHeight: form.implicitHeight + 88

        ColumnLayout {
            id: form
            x: 44
            y: 44
            width: Math.min(860, scroll.availableWidth - 88)
            spacing: 14

            Text { text: "New decision"; color: Theme.ink; font.pixelSize: 34; font.weight: Font.Bold }
            Text {
                Layout.fillWidth: true
                text: "You will be the organizer. Invited experts see the decision as soon as they sign in with their email."
                color: Theme.ink2
                wrapMode: Text.WordWrap
            }

            Card {
                Layout.fillWidth: true
                Layout.topMargin: 10
                spacing: 12

                Text { text: "Title"; color: Theme.ink; font.weight: Font.DemiBold }
                AppField { id: titleField; Layout.fillWidth: true; placeholderText: "e.g. Choosing a conference venue" }

                Text { text: "Description (optional)"; color: Theme.ink; font.weight: Font.DemiBold }
                AppField { id: descriptionField; Layout.fillWidth: true; placeholderText: "What is being decided and why" }

                // ---- Options ----
                SectionTitle { title: "Options"; hint: "What the group chooses between. At least two." }
                Repeater {
                    model: optionsModel
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        AppField {
                            Layout.fillWidth: true
                            placeholderText: "Option " + (index + 1)
                            text: model.name
                            onTextEdited: model.name = text
                        }
                        RemoveButton { enabled: optionsModel.count > 2; onClicked: optionsModel.remove(index) }
                    }
                }
                AppButton { text: "+ Add option"; onClicked: optionsModel.append({ name: "" }) }

                // ---- Criteria ----
                SectionTitle { title: "Criteria"; hint: "What the options are judged on. The weight says how much each criterion counts." }
                Repeater {
                    model: criteriaModel
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        AppField {
                            Layout.fillWidth: true
                            Layout.preferredWidth: 3
                            placeholderText: "e.g. Price (kPLN)"
                            text: model.name
                            onTextEdited: model.name = text
                        }
                        AppField {
                            Layout.preferredWidth: 80
                            horizontalAlignment: TextInput.AlignRight
                            text: model.weight
                            invalid: !(Number(text) >= 0)
                            onTextEdited: model.weight = text
                        }
                        Text {
                            Layout.preferredWidth: 44
                            text: { const t = page.totalWeight(criteriaModel); const w = Number(model.weight); return t > 0 && w > 0 ? Theme.pct(w / t) : "0%" }
                            color: Theme.muted
                        }
                        Segmented {
                            model: ["Higher is better", "Lower is better"]
                            currentIndex: isCost ? 1 : 0
                            onActivated: i => criteriaModel.setProperty(index, "isCost", i === 1)
                        }
                        RemoveButton { enabled: criteriaModel.count > 1; onClicked: criteriaModel.remove(index) }
                    }
                }
                AppButton { text: "+ Add criterion"; onClicked: criteriaModel.append({ name: "", weight: "1", isCost: false }) }

                // ---- Experts ----
                SectionTitle { title: "Experts"; hint: "People who score the options. The weight is how much influence each one has." }

                CheckBox {
                    id: meBox
                    checked: page.addMe
                    onToggled: page.addMe = checked
                    text: "Add me (" + page.session.email + ") as an expert"
                    contentItem: Text {
                        leftPadding: meBox.indicator.width + 10
                        text: meBox.text
                        color: Theme.ink
                        verticalAlignment: Text.AlignVCenter
                    }
                    indicator: Rectangle {
                        implicitWidth: 20; implicitHeight: 20
                        y: (meBox.height - height) / 2
                        radius: 5
                        color: meBox.checked ? Theme.ultra : "#FFFFFF"
                        border.color: meBox.checked ? Theme.ultra : Theme.line
                        Text { anchors.centerIn: parent; text: "✓"; color: "#FFFFFF"; visible: meBox.checked; font.weight: Font.Bold }
                    }
                }

                Repeater {
                    model: expertsModel
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        AppField {
                            Layout.fillWidth: true
                            Layout.preferredWidth: 2
                            placeholderText: "Name or role"
                            text: model.name
                            onTextEdited: model.name = text
                        }
                        AppField {
                            Layout.fillWidth: true
                            Layout.preferredWidth: 3
                            placeholderText: "Email"
                            text: model.email
                            onTextEdited: model.email = text
                        }
                        AppField {
                            Layout.preferredWidth: 80
                            horizontalAlignment: TextInput.AlignRight
                            text: model.weight
                            invalid: !(Number(text) >= 0)
                            onTextEdited: model.weight = text
                        }
                        RemoveButton { enabled: expertsModel.count > 1; onClicked: expertsModel.remove(index) }
                    }
                }
                AppButton { text: "+ Add expert"; onClicked: expertsModel.append({ name: "", email: "", weight: "1" }) }

                // ---- Target ----
                SectionTitle {
                    title: "Agreement target"
                    hint: "When agreement between experts reaches this level, the decision is considered settled. 1 means everyone ranks the options the same way."
                }
                RowLayout {
                    spacing: 16
                    Slider {
                        id: slider
                        Layout.preferredWidth: 320
                        from: 0.5; to: 1.0; stepSize: 0.05
                        value: page.target
                        onMoved: page.target = Math.round(value * 100) / 100
                        background: Rectangle {
                            x: slider.leftPadding
                            y: slider.topPadding + slider.availableHeight / 2 - height / 2
                            width: slider.availableWidth
                            height: 6
                            radius: 3
                            color: Theme.lineSoft
                            Rectangle { width: slider.visualPosition * parent.width; height: parent.height; radius: 3; color: Theme.ultra }
                        }
                        handle: Rectangle {
                            x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
                            y: slider.topPadding + slider.availableHeight / 2 - height / 2
                            width: 22; height: 22; radius: 11
                            color: "#FFFFFF"
                            border.color: Theme.ultra
                            border.width: 2
                        }
                    }
                    Text { text: Theme.fixed(page.target); color: Theme.ink; font.pixelSize: 18; font.weight: Font.Bold }
                }
            }

            RowLayout {
                Layout.topMargin: 6
                spacing: 12
                AppButton { kind: "primary"; text: "Create decision"; onClicked: page.submit() }
                AppButton { text: "Cancel"; onClicked: page.finished() }
            }
        }
    }
}
