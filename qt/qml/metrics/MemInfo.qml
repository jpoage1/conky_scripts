// qt/qml/metrics/MemInfo.qml
import QtQuick
import QtQuick.Layouts
import "../helpers"

ColumnLayout {
    Layout.alignment: Qt.AlignTop
    spacing: 4
    
    SectionHeader { text: "Memory"; Layout.leftMargin: 20 }
    
    // MEMORY SECTION
    MemoryBar { 
        label: "MEM"
        value: root.metrics ? root.metrics.meminfo.percent : 0 
    }
    DefaultText { 
        text: root.metrics ? ("Used: " + Math.round(root.metrics.meminfo.used_kb / 1024) + 
              " MB / Total: " + Math.round(root.metrics.meminfo.total_kb / 1024) + " MB") : ""
        Layout.leftMargin: 20 // Aligning with the Memory header offset
    }

    // SWAP SECTION
    MemoryBar { 
        label: "SWP"
        value: root.metrics ? root.metrics.swapinfo.percent : 0 
    }
    DefaultText { 
        text: root.metrics ? ("Used: " + Math.round(root.metrics.swapinfo.used_kb / 1024) + 
              " MB / Total: " + Math.round(root.metrics.swapinfo.total_kb / 1024) + " MB") : ""
        Layout.leftMargin: 20
    }
}
