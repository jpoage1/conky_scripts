import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root
    visible: true
    width: 700; height: 900
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysAtBottomHint

    // The data object provided by your SystemMetricsProxy
    property var systemData: backend.systemData

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.6)
        radius: 12
        border.color: "#00ccff"
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            // --- HEADER / SYSTEM INFO ---
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Text { text: "SYSTEM"; color: "#00ccff"; font.bold: true; font.pixelSize: 10 }
                    Text { text: "Host: " + (systemData ? systemData.node_name : "..."); color: "white"; font.pixelSize: 9 }
                    Text { text: "Temp: " + (systemData ? systemData.cpu_temp_c.toFixed(1) : "0") + "°C"; color: "white"; font.pixelSize: 9 }
                }

                // Memory Bar
                ColumnLayout {
                    Layout.fillWidth: true
                    Text { text: "MEMORY"; color: "#00ccff"; font.bold: true; font.pixelSize: 10 }
                    ProgressBar {
                        Layout.fillWidth: true
                        value: systemData ? systemData.meminfo.percent / 100 : 0
                        background: Rectangle { implicitHeight: 8; color: "#333"; radius: 4 }
                        contentItem: Item {
                            Rectangle {
                                width: parent.parent.visualPosition * parent.width
                                height: 8; radius: 4; color: "#0077ff"
                            }
                        }
                    }
                }
            }

            // --- CPU CORES (Stacked Bar Style) ---
            ColumnLayout {
                Layout.fillWidth: true
                Text { text: "CPU USAGE"; color: "#00ccff"; font.bold: true; font.pixelSize: 10 }
                
                Repeater {
                    model: systemData ? systemData.cores : 0
                    delegate: RowLayout {
                        Text { 
                            text: modelData.core_id === 0 ? "Avg" : "C" + (modelData.core_id-1)
                            color: "white"; font.pixelSize: 9; Layout.preferredWidth: 30 
                        }
                        Rectangle {
                            Layout.fillWidth: true; height: 12; color: "#333"; radius: 2; clip: true
                            Row {
                                anchors.fill: parent
                                Rectangle { width: parent.width * (modelData.user_percent/100); height: 12; color: "#77aaff" }
                                Rectangle { width: parent.width * (modelData.system_percent/100); height: 12; color: "#ff7777" }
                                Rectangle { width: parent.width * (modelData.iowait_percent/100); height: 12; color: "#ffaa77" }
                            }
                        }
                        Text { text: modelData.total_usage_percent.toFixed(1) + "%"; color: "white"; font.pixelSize: 9; Layout.preferredWidth: 40 }
                    }
                }
            }

            // --- PROCESS LIST ---
            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                header: Row {
                    spacing: 10
                    Text { text: "NAME"; width: 150; color: "#999999"; font.pixelSize: 9 }
                    Text { text: "PID"; width: 50; color: "#999999"; font.pixelSize: 9 }
                    Text { text: "CPU%"; width: 50; color: "#999999"; font.pixelSize: 9 }
                }
                model: systemData ? systemData.top_processes_real_cpu : []
                delegate: Row {
                    spacing: 10
                    Text { text: modelData.name; width: 150; color: "white"; font.pixelSize: 9; elide: Text.ElideRight }
                    Text { text: modelData.pid; width: 50; color: "white"; font.pixelSize: 9 }
                    Text { text: modelData.cpu_percent.toFixed(1) + "%"; width: 50; color: "#00ccff"; font.pixelSize: 9 }
                }
            }
        }
    }
}
