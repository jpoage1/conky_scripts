// qt/qml/windows/Telemetry.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../metrics"

Rectangle {
    id: mainWidget
    anchors.fill: parent
    color: Qt.rgba(0, 0, 0, 0.6)
    radius: 12
    border.color: root.cyan
    border.width: 1
    visible: root.metrics !== null

    ScrollView {
        anchors.fill: parent
        anchors.margins: 5
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        clip: true

        ColumnLayout {
            width: mainWidget.width - 40 
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 20
            spacing: 15

            RowLayout {
                Layout.fillWidth: true
                spacing: 20

                SystemInfo {}
                NetworkInfo {}
                MemInfo {}
            }

            CoreStat {}

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 180
                spacing: 20

                TopCPUProcesses {}
                TopMemProcesses {}
            }

            DiskUsage {}
            DiskIO {}
        }
    }
}
