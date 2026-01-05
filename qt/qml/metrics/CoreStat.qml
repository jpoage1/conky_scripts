// qt/qml/metrics/CoreStat.qml
import QtQuick
import QtQuick.Layouts
import "../helpers"

ColumnLayout {
    Layout.fillWidth: true
    spacing: 5
    SectionHeader { text: "CPU" }

    RowLayout {
        Layout.fillWidth: true
        spacing: 10
        DefaultText { text: "Core"; Layout.preferredWidth: 50; color: "#aaa"; font.bold: true }
        DefaultText { text: "User"; Layout.preferredWidth: 80; color: "#77aaff"; font.bold: true; horizontalAlignment: Text.AlignHCenter }
        DefaultText { text: "System"; Layout.preferredWidth: 80; color: "#ff7777"; font.bold: true; horizontalAlignment: Text.AlignHCenter }
        DefaultText { text: "Nice"; Layout.preferredWidth: 80; color: "#77ff77"; font.bold: true; horizontalAlignment: Text.AlignHCenter }
        DefaultText { text: "IOWait"; Layout.preferredWidth: 80; color: "#ffaa77"; font.bold: true; horizontalAlignment: Text.AlignHCenter }
        DefaultText { text: "Idle"; Layout.preferredWidth: 80; color: "#999"; font.bold: true; horizontalAlignment: Text.AlignHCenter }
        DefaultText { text: "Total"; Layout.preferredWidth: 50; color: "#aaa"; font.bold: true; horizontalAlignment: Text.AlignRight }
    }

    Repeater {
        model: root.metrics ? root.metrics.cores : []
        delegate: RowLayout {
            Layout.fillWidth: true
            spacing: 10
            DefaultText { text: modelData.core_id === 0 ? "Avg" : "Core " + modelData.core_id; Layout.preferredWidth: 50 }
            MetricBar { value: modelData.user_percent; barColor: "#77aaff" }
            MetricBar { value: modelData.system_percent; barColor: "#ff7777" }
            MetricBar { value: modelData.nice_percent; barColor: "#77ff77" }
            MetricBar { value: modelData.iowait_percent; barColor: "#ffaa77" }
            MetricBar { value: modelData.idle_percent; barColor: "transparent" }
            DefaultText { text: modelData.total_usage_percent.toFixed(1) + "%"; Layout.preferredWidth: 50; horizontalAlignment: Text.AlignRight }
        }
    }
}
