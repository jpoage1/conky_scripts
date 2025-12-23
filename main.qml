// main.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    visible: true
    width: 700; height: 900
    color: "#1e1e1e"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysAtBottomHint // Desktop mode

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        // TOP ROW: System, Network, Memory
        RowLayout {
            Layout.fillWidth: true
            spacing: 15

            // System Info
            Column {
                Layout.preferredWidth: parent.width / 3
                Text { text: "System"; font.bold: true; color: "cyan" }
                Text { text: "Host: " + systemData.node_name; color: "white" }
                Text { text: "Uptime: " + systemData.uptime; color: "white" }
                Text { text: "Temp: " + systemData.cpu_temp_c + "°C"; color: "white" }
            }

            // Network
            Column {
                Layout.preferredWidth: parent.width / 3
                Text { text: "Network"; font.bold: true; color: "cyan" }
                Repeater {
                    model: systemData.network_interfaces
                    Text { 
                        text: modelData.interface_name + ": ↓" + modelData.rx_bytes_per_sec; 
                        color: "white" 
                    }
                }
            }

            // Memory/Swap
            Column {
                Layout.preferredWidth: parent.width / 3
                Text { text: "Memory"; font.bold: true; color: "cyan" }
                CustomBar { label: "MEM"; value: systemData.meminfo.percent }
                CustomBar { label: "SWP"; value: systemData.swapinfo.percent }
            }
        }

        // CPU CORES (The Stacked Bar equivalent)
        ColumnLayout {
            Text { text: "CPU Cores"; font.bold: true; color: "cyan" }
            Repeater {
                model: systemData.cores
                RowLayout {
                    Text { text: modelData.core_id === 0 ? "Avg" : "Core " + (modelData.core_id-1); color: "white"; width: 50 }
                    Rectangle {
                        Layout.fillWidth: true; height: 18; color: "#333"
                        Row {
                            Rectangle { width: parent.width * (modelData.user_percent/100); height: 18; color: "green" }
                            Rectangle { width: parent.width * (modelData.system_percent/100); height: 18; color: "red" }
                            Rectangle { width: parent.width * (modelData.iowait_percent/100); height: 18; color: "orange" }
                        }
                    }
                }
            }
        }

        // PROCESS LIST (TableView)
        ListView {
            Layout.fillWidth: true; Layout.fillHeight: true
            header: Row {
                Text { text: "Name"; width: 140; color: "gray" }
                Text { text: "PID"; width: 70; color: "gray" }
                Text { text: "CPU"; width: 70; color: "gray" }
            }
            model: systemData.top_processes_real_cpu
            delegate: Row {
                Text { text: modelData.name; width: 140; color: "white"; elide: Text.ElideRight }
                Text { text: modelData.pid; width: 70; color: "white" }
                Text { text: modelData.cpu_percent + "%"; width: 70; color: "white" }
            }
        }
    }
}
