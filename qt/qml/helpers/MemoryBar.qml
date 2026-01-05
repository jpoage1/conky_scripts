// qt/qml/helpers/MemoryBar.qml
import QtQuick
import QtQuick.Layouts

RowLayout {
    property string label: ""
    property real value: 0
    spacing: 10

    DefaultText { text: label; Layout.preferredWidth: 40 }

    Rectangle {
        Layout.preferredWidth: 150
        Layout.preferredHeight: 12
        color: root.bgSubtle
        radius: 6
        clip: true

        Rectangle {
            width: parent.width * (value / 100)
            height: parent.height
            radius: 6
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "#66b3ff" }
                GradientStop { position: 1.0; color: "#0077ff" }
            }
        }
    }
    DefaultText { text: value.toFixed(0) + "%" }
}
