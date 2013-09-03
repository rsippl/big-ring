import QtQuick 2.0

Rectangle {
    width: textBox.paintedWidth + 10
    height: textBox.paintedHeight + 10
    color: "transparent"
    property string textColor: "red"
    property string text: ""

    Rectangle {
        anchors.fill: parent
        color: "black"
        radius: 10
        opacity: 0.5
    }

    Text {
        id: textBox
        anchors.centerIn: parent
        color: parent.textColor;
        font.pointSize: 36
        font.bold: true
        text: parent.text
    }

}
