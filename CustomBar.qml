// CustomBar.qml
Row {
    property string label: ""
    property real value: 0
    spacing: 10
    Text { text: label; width: 40; color: "white" }
    ProgressBar {
        value: parent.value / 100
        width: 150
    }
    Text { text: parent.value + "%"; color: "white" }
}
