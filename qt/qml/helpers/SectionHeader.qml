// qt/qml/helpers/SectionHeader.qml
import QtQuick
import QtQuick.Layouts

Text {
        // These link to root properties in main.qml via dynamic scoping
    font.family: root.monoFont
    color: root.cyan
    font.pixelSize: root.headerFontSize
    font.bold: true
    Layout.leftMargin: 100 
    Layout.bottomMargin: 5
}
