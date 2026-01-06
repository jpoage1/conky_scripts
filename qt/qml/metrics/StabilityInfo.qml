// qt/qml/metrics/StabilityInfo.qml
import QtQuick
import QtQuick.Layouts
import "../helpers"

ColumnLayout {
    Layout.alignment: Qt.AlignTop
    spacing: 3
    SectionHeader { text: "System Stability"; Layout.leftMargin: 20 }

    readonly property var stability: root.metrics ? root.metrics.stability : null

    // Pressure Stall Information (PSI)
    DefaultText { 
        text: "Mem Pressure: " + (stability ? (stability.pressure.memory.some.toFixed(2) + "%") : "0.00%") 
        color: (stability && stability.pressure.memory.some > 5) ? "orange" : root.cyan
    }
    DefaultText { 
        text: "IO Pressure:  " + (stability ? (stability.pressure.io.some.toFixed(2) + "%") : "0.00%") 
    }

    // File Descriptors
    DefaultText { 
        text: "Open FDs:     " + (stability ? (stability.file_descriptors.allocated + " / " + stability.file_descriptors.max) : "0 / 0")
        color: (stability && (stability.file_descriptors.allocated / stability.file_descriptors.max) > 0.8) ? "red" : root.cyan
    }

    // Fragmentation Index
    DefaultText { 
        text: "RAM Frag:     " + (stability ? stability.memory_fragmentation_index.toFixed(3) : "0.000")
        color: (stability && stability.memory_fragmentation_index > 0.7) ? "yellow" : root.cyan
    }
}
