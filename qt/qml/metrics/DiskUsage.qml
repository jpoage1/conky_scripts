// qt/qml/metrics/DiskUsage.qml
import QtQuick
import QtQuick.Layouts
import "../helpers"

ColumnLayout {
    Layout.fillWidth: true
    spacing: 5
    SectionHeader { text: "Disk Usage (GiB)" }

    RowLayout {
        Layout.fillWidth: true; spacing: 10
        DefaultText { text: "Mount"; Layout.preferredWidth: 150; color: "#aaa"; font.bold: true }
        DefaultText { text: "Total"; Layout.preferredWidth: 60; color: "#aaa"; font.bold: true; horizontalAlignment: Text.AlignRight }
        DefaultText { text: "Used";  Layout.preferredWidth: 60; color: "#aaa"; font.bold: true; horizontalAlignment: Text.AlignRight }
        DefaultText { text: "Free";  Layout.preferredWidth: 60; color: "#aaa"; font.bold: true; horizontalAlignment: Text.AlignRight }
        DefaultText { text: "Used %"; Layout.preferredWidth: 70; color: "#aaa"; font.bold: true; horizontalAlignment: Text.AlignRight }
    }

    Repeater {
        model: root.metrics ? root.metrics.disks : []
        delegate: RowLayout {
            Layout.fillWidth: true; spacing: 10
            visible: modelData.usage.size_bytes > 0
            DefaultText { text: modelData.mount_point; Layout.preferredWidth: 150; elide: Text.ElideMiddle }
            DefaultText { text: (modelData.usage.size_bytes / 1073741824).toFixed(1); Layout.preferredWidth: 60; horizontalAlignment: Text.AlignRight }
            DefaultText { text: (modelData.usage.used_bytes / 1073741824).toFixed(1); Layout.preferredWidth: 60; horizontalAlignment: Text.AlignRight }
            DefaultText { text: ((modelData.usage.size_bytes - modelData.usage.used_bytes) / 1073741824).toFixed(1); Layout.preferredWidth: 60; color: root.cyan; horizontalAlignment: Text.AlignRight }
            DefaultText { text: ((modelData.usage.used_bytes / modelData.usage.size_bytes) * 100).toFixed(2) + "%"; Layout.preferredWidth: 70; horizontalAlignment: Text.AlignRight }
        }
    }
}
