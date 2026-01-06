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
            DefaultText {
                text: "fd:" + modelData.open_fds;
                Layout.preferredWidth: 40;
                color: modelData.open_fds > 500 ? "orange" : "#aaa"
            }

            DefaultText {
                text: "w:" + (modelData.io_write_bytes / 1024).toFixed(0) + "k";
                Layout.preferredWidth: 60;
                horizontalAlignment: Text.AlignRight
            }
            DefaultText { text: modelData.cpu_percent.toFixed(1) + "%"; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
        }
    }
}
