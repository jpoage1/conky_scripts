// qt/qml/helpers/MetricBar.qml
import QtQuick
import QtQuick.Layouts

Rectangle {
    property real value: 0
    property color barColor: "white"

    Layout.preferredWidth: 80
    Layout.preferredHeight: 14
    color: root.bgSubtle
    radius: 4
    clip: true

    Rectangle {
        width: parent.width * (parent.value / 100)
        height: parent.height
        color: barColor
        radius: 2
    }
}
