// qt/qml/main.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "helpers"
import "metrics"
import "windows"

Window {
    id: root
    visible: true
    
    // Set initial size
    width: 800 
    height: 950
    
    // Set minimums to prevent layout collapse
    minimumWidth: 400
    minimumHeight: 300

    color: "transparent"
    
    // Note: FramelessWindowHint often disables OS-level resizing. 
    // If you want to keep the frame gone, you must handle resizing via QML 
    // or use a window manager that allows it (like tiling WMs).
    flags: Qt.FramelessWindowHint | Qt.WindowStaysAtBottomHint

    readonly property var metrics: (systemData && systemData.data && systemData.data.length > 0) 
                                   ? systemData.data[0] 
                                   : null

    readonly property string monoFont: "DejaVu Sans Mono"
    readonly property color cyan: "#00ccff"
    readonly property color bgSubtle: "#333333"
    readonly property int defaultFontSize: 7 
    readonly property int headerFontSize: 10 

    Telemetry {
        // Force the Telemetry component to follow the Window's size
        anchors.fill: parent
        visible: root.metrics !== null
    }

    Splash {
        anchors.fill: parent
        visible: root.metrics === null
    }
}
