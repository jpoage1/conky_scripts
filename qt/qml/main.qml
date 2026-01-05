import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root
    visible: true
    width: 700
    height: 900
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysAtBottomHint

    readonly property var metrics: (systemData && systemData.data && systemData.data.length > 0) 
                                   ? systemData.data[0] 
                                   : null

    readonly property color cyan: "#00ccff"
    readonly property color bgSubtle: "#333333"
    readonly property string monoFont: "DejaVu Sans Mono"

    Rectangle {
        anchors.fill: parent
        color: "#111"
        visible: root.metrics === null
        Text {
            anchors.centerIn: parent
            text: "Initializing Telemetry..."
            color: root.cyan
            font.family: root.monoFont
        }
    }

    Rectangle {
        id: mainWidget
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.6)
        radius: 12
        border.color: root.cyan
        border.width: 1
        visible: root.metrics !== null

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            // FIXED: Use explicit Layout.fillWidth and Layout.preferredWidth instead of parent.width
            RowLayout {
                Layout.fillWidth: true
                spacing: 15

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1  // Equal weight
                    Text { text: "System"; font.pixelSize: 10; font.bold: true; color: root.cyan }
                    Text { text: "Host:   " + (root.metrics ? root.metrics.node_name : ""); font.family: root.monoFont; font.pixelSize: 9; color: "white" }
                    Text { text: "Kernel: " + (root.metrics ? (root.metrics.sys_name + " " + root.metrics.machine_type) : ""); font.family: root.monoFont; font.pixelSize: 9; color: "white" }
                    Text { text: "Uptime: " + (root.metrics ? root.metrics.uptime.text : ""); font.family: root.monoFont; font.pixelSize: 9; color: "white" }
                    Text { text: "Freq:   " + (root.metrics ? root.metrics.cpu_frequency_ghz.toFixed(2) : "0.00") + " GHz"; font.family: root.monoFont; font.pixelSize: 9; color: "white" }
                    Text { text: "Temp:   " + (root.metrics ? root.metrics.cpu_temp_c.toFixed(1) : "0.0") + "°C"; font.family: root.monoFont; font.pixelSize: 9; color: "white" }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1  // Equal weight
                    Text { text: "Network (Raw Bytes/s)"; font.pixelSize: 10; font.bold: true; color: root.cyan }
                    Row {
                        spacing: 10
                        Text { text: "Device"; width: 70; color: "#aaa"; font.bold: true; font.pixelSize: 9 }
                        Text { text: "Down";   width: 70; color: "#aaa"; font.bold: true; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                        Text { text: "Up";     width: 70; color: "#aaa"; font.bold: true; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                    }
                    Repeater {
                        model: root.metrics ? root.metrics.network_interfaces : []
                        delegate: Row {
                            spacing: 10
                            Text { text: modelData.interface_name; width: 70; color: "white"; font.pixelSize: 9 }
                            Text { text: modelData.rx_bytes_per_sec.toFixed(1); width: 70; color: "white"; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                            Text { text: modelData.tx_bytes_per_sec.toFixed(1); width: 70; color: "white"; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1  // Equal weight
                    Text { text: "Memory"; font.pixelSize: 10; font.bold: true; color: root.cyan }
                    
                    Row {
                        spacing: 5
                        Text { text: "MEM"; color: "white"; font.pixelSize: 9; width: 30 }
                        Rectangle {
                            width: 120; height: 10; color: root.bgSubtle; radius: 5
                            Rectangle {
                                width: (root.metrics && root.metrics.meminfo) ? (parent.width * (root.metrics.meminfo.percent / 100)) : 0
                                height: 10; radius: 5; color: "#0077ff"
                            }
                        }
                        Text { text: (root.metrics && root.metrics.meminfo ? root.metrics.meminfo.percent : 0) + "%"; color: "white"; font.pixelSize: 9 }
                    }
                    Text { 
                        text: root.metrics && root.metrics.meminfo ? ("Used: " + (root.metrics.meminfo.used_kb/1024).toFixed(0) + "MB / " + (root.metrics.meminfo.total_kb/1024).toFixed(0) + "MB") : ""
                        color: "white"; font.pixelSize: 8; opacity: 0.8 
                    }

                    Row {
                        topPadding: 5; spacing: 5
                        Text { text: "SWP"; color: "white"; font.pixelSize: 9; width: 30 }
                        Rectangle {
                            width: 120; height: 10; color: root.bgSubtle; radius: 5
                            Rectangle {
                                width: (root.metrics && root.metrics.swapinfo) ? (parent.width * (root.metrics.swapinfo.percent / 100)) : 0
                                height: 10; radius: 5; color: "#0077ff"
                            }
                        }
                        Text { text: (root.metrics && root.metrics.swapinfo ? root.metrics.swapinfo.percent : 0) + "%"; color: "white"; font.pixelSize: 9 }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Text { text: "CPU"; font.pixelSize: 10; font.bold: true; color: root.cyan }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Core"; Layout.preferredWidth: 50; color: "#aaa"; font.pixelSize: 9 }
                    Row {
                        Layout.fillWidth: true; spacing: 15
                        Text { text: "User";   color: "#77aaff"; font.pixelSize: 8; font.bold: true }
                        Text { text: "System"; color: "#ff7777"; font.pixelSize: 8; font.bold: true }
                        Text { text: "Nice";   color: "#77ff77"; font.pixelSize: 8; font.bold: true }
                        Text { text: "IOWait"; color: "#ffaa77"; font.pixelSize: 8; font.bold: true }
                        Text { text: "Idle";   color: "#999";    font.pixelSize: 8; font.bold: true }
                    }
                    Text { text: "Total"; Layout.preferredWidth: 40; color: "#aaa"; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                }

                Repeater {
                    model: root.metrics ? root.metrics.cores : []
                    delegate: RowLayout {
                        Layout.fillWidth: true
                        Text { 
                            text: modelData.core_id === 0 ? "Avg" : "Core " + (modelData.core_id - 1)
                            Layout.preferredWidth: 50; color: "white"; font.pixelSize: 9 
                        }
                        Rectangle {
                            Layout.fillWidth: true; height: 14; color: root.bgSubtle; radius: 4; clip: true
                            Row {
                                anchors.fill: parent
                                Rectangle { width: parent.width * (modelData.user_percent/100);   height: parent.height; color: "#77aaff" }
                                Rectangle { width: parent.width * (modelData.system_percent/100); height: parent.height; color: "#ff7777" }
                                Rectangle { width: parent.width * (modelData.nice_percent/100);   height: parent.height; color: "#77ff77" }
                                Rectangle { width: parent.width * (modelData.iowait_percent/100); height: parent.height; color: "#ffaa77" }
                            }
                        }
                        Text { 
                            text: modelData.total_usage_percent.toFixed(1) + "%"
                            Layout.preferredWidth: 40; color: "white"; font.pixelSize: 9; horizontalAlignment: Text.AlignRight 
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true; spacing: 5
                Text { text: "Top Realtime Processes"; font.pixelSize: 10; font.bold: true; color: root.cyan }
                Row {
                    spacing: 10
                    Text { text: "Name"; width: 140; color: "#aaa"; font.bold: true; font.pixelSize: 9 }
                    Text { text: "PID";  width: 70;  color: "#aaa"; font.bold: true; font.pixelSize: 9 }
                    Text { text: "CPU";  width: 70;  color: "#aaa"; font.bold: true; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                    Text { text: "MEM";  width: 70;  color: "#aaa"; font.bold: true; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                }
                ListView {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    model: root.metrics ? root.metrics.top_processes_real_cpu : []
                    interactive: false 
                    delegate: Row {
                        spacing: 10
                        Text { text: modelData.name; width: 140; color: "white"; font.pixelSize: 9; elide: Text.ElideRight }
                        Text { text: modelData.pid;  width: 70;  color: "white"; font.pixelSize: 9 }
                        Text { text: modelData.cpu_percent.toFixed(1) + "%"; width: 70; color: root.cyan; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                        Text { text: modelData.mem_percent.toFixed(1) + "%"; width: 70; color: "white"; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true; spacing: 5
                Text { text: "Disk Usage (GiB)"; font.pixelSize: 10; font.bold: true; color: root.cyan }
                Row {
                    spacing: 10
                    Text { text: "Mount"; width: 120; color: "#aaa"; font.pixelSize: 9 }
                    Text { text: "Total"; width: 50;  color: "#aaa"; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                    Text { text: "Used";  width: 50;  color: "#aaa"; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                    Text { text: "Free";  width: 50;  color: "#aaa"; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                }
                Repeater {
                    model: root.metrics ? root.metrics.disks : []
                    delegate: Row {
                        visible: modelData.usage.size_bytes > 0 
                        spacing: 10
                        Text { text: modelData.mount_point || "---"; width: 120; color: "white"; font.pixelSize: 9; elide: Text.ElideMiddle }
                        Text { text: (modelData.usage.size_bytes / 1073741824).toFixed(1); width: 50; color: "white"; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                        Text { text: (modelData.usage.used_bytes / 1073741824).toFixed(1); width: 50; color: "white"; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                        Text { text: ((modelData.usage.size_bytes - modelData.usage.used_bytes) / 1073741824).toFixed(1); width: 50; color: root.cyan; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                    }
                }
            }
        }
    }
}
