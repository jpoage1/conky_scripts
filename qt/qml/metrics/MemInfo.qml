// qt/qml/metrics/MemInfo.qml
import QtQuick
import QtQuick.Layouts
import "../helpers"

ColumnLayout {
    Layout.alignment: Qt.AlignTop
    spacing: 4
    SectionHeader { text: "Memory"; Layout.leftMargin: 20 }
    MemoryBar { label: "MEM"; value: root.metrics ? root.metrics.meminfo.percent : 0 }
    MemoryBar { label: "SWP"; value: root.metrics ? root.metrics.swapinfo.percent : 0 }
}
