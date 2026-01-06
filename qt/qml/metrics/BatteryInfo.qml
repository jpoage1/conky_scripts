// qt/qml/metrics/BatteryInfo.qml
import QtQuick
import QtQuick.Layouts
import "../helpers"

ColumnLayout {
    Layout.alignment: Qt.AlignTop
    spacing: 3
    SectionHeader { text: "Power"; Layout.leftMargin: 20 }

    Repeater {
        model: root.metrics ? root.metrics.battery_info : []
        delegate: RowLayout {
            spacing: 10
            
            // Battery Icon Logic
            Rectangle {
                width: 20; height: 10
                radius: 2
                color: "transparent"
                border.color: modelData.status === "Charging" ? "#77ff77" : root.cyan
                border.width: 1

                // Battery Fill
                Rectangle {
                    width: (parent.width - 2) * (modelData.percentage / 100)
                    height: parent.height - 2
                    anchors.centerIn: parent
                    color: modelData.percentage < 20 ? "#ff4444" : 
                           modelData.status === "Charging" ? "#77ff77" : root.cyan
                }
                
                // Battery Tip
                Rectangle {
                    width: 2; height: 4
                    anchors.left: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    color: parent.border.color
                }
            }

            DefaultText { 
                text: modelData.name + ": " + modelData.percentage + "% (" + modelData.status + ")"
                font.bold: modelData.status === "Charging"
            }
        }
    }
}
