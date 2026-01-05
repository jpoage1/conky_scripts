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
    
    width: 800 
    height: 950
    minimumWidth: 800
    maximumWidth: 800
    minimumHeight: 950
    maximumHeight: 950
    
    color: "transparent"
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
        visible: root.metrics !== null
    }

    Splash {
        visible: root.metrics === null
    }
}
