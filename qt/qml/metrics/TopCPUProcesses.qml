// qt/qml/metrics/TopCPUProcesses.qml
import QtQuick
import QtQuick.Layouts
import "../helpers"

ColumnLayout {
    Layout.fillWidth: true
    SectionHeader { text: "Top CPU"; Layout.leftMargin: 0 }
    ListView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        interactive: false
        model: root.metrics ? root.metrics.top_processes_real_cpu : []
        delegate: RowLayout {
            spacing: 10
            DefaultText { text: modelData.name; Layout.preferredWidth: 120; elide: Text.ElideRight }
            DefaultText { text: modelData.pid; Layout.preferredWidth: 50 }
            DefaultText { text: modelData.cpu_percent.toFixed(1) + "%"; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
        }
    }
}
