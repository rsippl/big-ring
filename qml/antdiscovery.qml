import QtQuick 2.0

Rectangle {
    id: main
    width: 600
    height: 300

    Connections {
        target: antDeviceFinder
        onDeviceFound: console.log("device found", type)
    }

    Text {
        id: name
        text: qsTr("Hoi")
    }


    Rectangle {
        width: 100
        height: 30
        color: "blue"
        anchors.bottom: main.bottom
        anchors.horizontalCenter: main.horizontalCenter
        Text {
            text: qsTr("Start Search")
        }

        MouseArea {
            id: searchButtonArea
            anchors.fill: parent
            onPressed: antDeviceFinder.startSearch()
        }
    }
}
