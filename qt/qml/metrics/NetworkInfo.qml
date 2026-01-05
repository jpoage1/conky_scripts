// qt/qml/metrics/NetworkInfo.qml
import QtQuick
import QtQuick.Layouts
import "../helpers"

ColumnLayout {
    Layout.alignment: Qt.AlignTop
    spacing: 3
    SectionHeader { text: "Network"; Layout.leftMargin: 20 }
    RowLayout {
        spacing: 10
        DefaultText { text: "Device"; Layout.preferredWidth: 70; color: "#aaa"; font.bold: true }
        DefaultText { text: "Down";   Layout.preferredWidth: 70; color: "#aaa"; font.bold: true; horizontalAlignment: Text.AlignRight }
        DefaultText { text: "Up";     Layout.preferredWidth: 70; color: "#aaa"; font.bold: true; horizontalAlignment: Text.AlignRight }
    }
    Repeater {
        model: root.metrics ? root.metrics.network_interfaces : []
        delegate: RowLayout {
            spacing: 10
            DefaultText { text: modelData.interface_name; Layout.preferredWidth: 70 }
            DefaultText { text: modelData.rx_bytes_per_sec.toFixed(1); Layout.preferredWidth: 70; horizontalAlignment: Text.AlignRight }
            DefaultText { text: modelData.tx_bytes_per_sec.toFixed(1); Layout.preferredWidth: 70; horizontalAlignment: Text.AlignRight }
        }
    }
}
