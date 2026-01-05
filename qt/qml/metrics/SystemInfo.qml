// qt/qml/metrics/SystemInfo.qml
import QtQuick
import QtQuick.Layouts
import "../helpers"

ColumnLayout {
    Layout.alignment: Qt.AlignTop
    spacing: 3
    
    SectionHeader { text: "System" }
    
    DefaultText { text: "Host:   " + (root.metrics ? root.metrics.node_name : "") }
    DefaultText { text: "Kernel: " + (root.metrics ? (root.metrics.sys_name + " " + root.metrics.machine_type + " " + root.metrics.kernel_release) : "") }
    DefaultText { text: "Uptime: " + (root.metrics ? root.metrics.uptime.text : "") }
    DefaultText { text: "Freq:   " + (root.metrics ? root.metrics.cpu_frequency_ghz.toFixed(2) : "0.00") + " GHz" }
    DefaultText { text: "Temp:   " + (root.metrics ? root.metrics.cpu_temp_c.toFixed(1) : "0.0") + "°C" }
    
    DefaultText { 
        text: "Total: " + (root.metrics ? root.metrics.processes_total : "0") + 
              " | Running: " + (root.metrics ? root.metrics.processes_running : "0") 
    }
    
    DefaultText { 
        text: "Load Average: 1m: " + (root.metrics ? root.metrics.load_avg_1m.toFixed(2) : "0.00") + 
              " | 5m: " + (root.metrics ? root.metrics.load_avg_5m.toFixed(2) : "0.00") + 
              " | 15m: " + (root.metrics ? root.metrics.load_avg_15m.toFixed(2) : "0.00")
    }
}
