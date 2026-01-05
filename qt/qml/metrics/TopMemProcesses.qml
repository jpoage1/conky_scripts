// qt/qml/metrics/TopMemProcesses.qml
import QtQuick
import QtQuick.Layouts
import "../helpers"

ColumnLayout {
    Layout.fillWidth: true
    SectionHeader { text: "Top Mem"; Layout.leftMargin: 0 }
    ListView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        interactive: false
        model: root.metrics ? root.metrics.top_processes_real_mem : []
        delegate: RowLayout {
            spacing: 10
            DefaultText { text: modelData.name; Layout.preferredWidth: 120; elide: Text.ElideRight }
            DefaultText { text: modelData.pid; Layout.preferredWidth: 50 }
            DefaultText { text: modelData.mem_percent.toFixed(1) + "%"; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
        }
    }
}
