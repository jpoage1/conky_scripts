// qt/qml/windows/Splash.qml
import QtQuick
import QtQuick.Layouts
import "../helpers"

Rectangle {
    anchors.fill: parent
    color: "#111"
    DefaultText {
        anchors.centerIn: parent
        text: "Initializing Telemetry..."
        font.pointSize: 12
    }
}
