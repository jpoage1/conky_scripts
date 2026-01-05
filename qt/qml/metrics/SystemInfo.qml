// qt/qml/metrics/SystemInfo.qml
import QtQuick
import QtQuick.Layouts
import "../helpers"

ColumnLayout {
    Layout.alignment: Qt.AlignTop
    spacing: 3
    SectionHeader { text: "System" }
    DefaultText { text: "Host:   " + (root.metrics ? root.metrics.node_name : "") }
    DefaultText { text: "Kernel: " + (root.metrics ? (root.metrics.sys_name + " " + root.metrics.machine_type) : "") }
    DefaultText { text: "Uptime: " + (root.metrics ? root.metrics.uptime.text : "") }
    DefaultText { text: "Freq:   " + (root.metrics ? root.metrics.cpu_frequency_ghz.toFixed(2) : "0.00") + " GHz" }
    DefaultText { text: "Temp:   " + (root.metrics ? root.metrics.cpu_temp_c.toFixed(1) : "0.0") + "°C" }
}
