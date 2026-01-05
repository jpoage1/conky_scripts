// qt/qml/metrics/DiskIO.qml
import QtQuick
import QtQuick.Layouts
import "../helpers"

ColumnLayout {
    Layout.fillWidth: true
    spacing: 3
    SectionHeader { text: "Disk I/O (KiB/s)" }
    RowLayout {
        spacing: 10
        DefaultText { text: "Device"; Layout.preferredWidth: 100; color: "#aaa"; font.bold: true }
        DefaultText { text: "Read kb/s"; Layout.preferredWidth: 100; color: "#aaa"; font.bold: true; horizontalAlignment: Text.AlignRight }
        DefaultText { text: "Write kb/s"; Layout.preferredWidth: 100; color: "#aaa"; font.bold: true; horizontalAlignment: Text.AlignRight }
    }
    Repeater {
        model: root.metrics ? root.metrics.disk_io : []
        delegate: RowLayout {
            spacing: 10
            DefaultText { text: modelData[0]; Layout.preferredWidth: 100; elide: Text.ElideRight }
            DefaultText { text: (modelData[1].read_bytes_per_sec / 1024).toFixed(2); Layout.preferredWidth: 100; horizontalAlignment: Text.AlignRight }
            DefaultText { text: (modelData[1].write_bytes_per_sec / 1024).toFixed(2); Layout.preferredWidth: 100; horizontalAlignment: Text.AlignRight }
        }
    }
}
