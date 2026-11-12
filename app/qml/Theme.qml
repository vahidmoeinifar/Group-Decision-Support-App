pragma Singleton
import QtQuick

// Colors, sizes and small formatting helpers shared by every screen.
QtObject {
    // Base colors
    readonly property color ink: "#19213A"         // dark navy: sidebar, headings
    readonly property color ink2: "#46506B"        // body text
    readonly property color muted: "#7A849B"       // hints, labels
    readonly property color mist: "#E9EDF3"        // page background
    readonly property color paper: "#F8FAFC"       // cards
    readonly property color line: "#D6DCE6"
    readonly property color lineSoft: "#E3E8EF"

    // Accents
    readonly property color ultra: "#3346D3"       // main action color
    readonly property color ultraDark: "#2737B0"
    readonly property color ultraSoft: "#E4E8FB"
    readonly property color saffron: "#E8A33B"     // winners, highlights
    readonly property color saffronSoft: "#FBEFD9"
    readonly property color green: "#1F8A5B"
    readonly property color greenSoft: "#E3F4EC"
    readonly property color red: "#C4314B"
    readonly property color redSoft: "#FBE7EA"

    // Sidebar
    readonly property color sideItem: "#273050"
    readonly property color sideText: "#C9D0E0"
    readonly property color sideMuted: "#8C95AD"

    // One color per option in charts
    readonly property var optionColors: ["#5468E8", "#E0577A", "#2BB59A", "#E8A33B",
                                         "#8B5CF6", "#0EA5E9", "#F97316", "#64748B"]

    // Sizes
    readonly property int radius: 14
    readonly property int gap: 20
    readonly property int bodySize: 14
    readonly property int smallSize: 12

    function optionColor(index) {
        return optionColors[((index % optionColors.length) + optionColors.length) % optionColors.length]
    }

    function pct(x) {
        return Math.round(x * 100) + "%"
    }

    // Short number: 6.5 not 6.5000, 120 not 120.00
    function num(x, digits) {
        if (x === undefined || x === null || isNaN(x))
            return "–"
        const d = digits === undefined ? 2 : digits
        return String(Number(Number(x).toFixed(d)))
    }

    // Fixed number of decimals, e.g. agreement 0.90
    function fixed(x, digits) {
        if (x === undefined || x === null || isNaN(x))
            return "–"
        return Number(x).toFixed(digits === undefined ? 2 : digits)
    }

    function agreementLabel(x) {
        if (x === null || x === undefined) return "Not enough experts"
        if (x >= 0.8) return "Strong agreement"
        if (x >= 0.5) return "Moderate agreement"
        if (x >= 0.0) return "Weak agreement"
        return "Disagreement"
    }

    function methodLabel(m) {
        return m === "Topsis" ? "TOPSIS" : m === "WeightedSum" ? "Weighted sum" : "Borda count"
    }
}
