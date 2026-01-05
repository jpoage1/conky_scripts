// qt/qml/helpers/DefaultText.qml
import QtQuick

Text {
    // These link to root properties in main.qml via dynamic scoping
    font.family: root.monoFont
    font.pointSize: root.defaultFontSize
    color: root.cyan
}
