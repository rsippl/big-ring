import QtQuick 2.0
import QtMultimedia 5.0

Rectangle {
    color: "black"

    MediaPlayer {
        id: player
        source: "file:///home/ibooij/Downloads/TheSufferfestRevolver2013 (1)/TheSuffefestRevolver2013.mp4"
    }

    VideoOutput {
        id: videoOutput
        source: player
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectCrop

        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.debug("clicked")
                videoOutput.togglePlay()
            }
        }
        function togglePlay() {
            if (player.playbackState === MediaPlayer.PlayingState) {
                player.pause();
                runTimer.stop();

            } else {
                game.start();
                player.play();
                runTimer.start();
            }
        }

        focus: true
        Keys.onSpacePressed: togglePlay()
        Keys.onLeftPressed: player.seek(player.position - 60000)
        Keys.onRightPressed: player.seek(player.position + 60000)
    }
    Rectangle {
        width: clockText.paintedWidth + 20
        height: clockText.paintedHeight + 10
        x: parent.width / 2 - width / 2
        y: 0
        z: 1
        color: "transparent"
        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.5
            radius: 10

        }

        Text {
            id: clockText
            anchors.centerIn: parent
            color: "white"
            text: "00:00:00"
            font.pointSize: 48
            opacity: 1.0
        }
    }

    Row {
        id: metrics

        width: parent.width
        anchors.bottom: parent.bottom

        Gauge {
            width: metrics.width / metrics.children.length
            text: ant.heartRate
            textColor: "blue"
        }
        Gauge {
            width: metrics.width / metrics.children.length
            text: "300 W"
            textColor: "red"
        }
        Gauge {
            width: metrics.width / metrics.children.length
            text: "93 RPM"
            textColor: "pink"
        }
    }

    property variant startTime: null

    function timeChanged() {
        clockText.text = Qt.formatTime(game.gameTime, "HH:mm:ss");
        console.debug(ant.heartRate)
    }

    Timer {
        id: runTimer
        interval: 100; running: false; repeat: true;
        onTriggered: parent.timeChanged()
    }
}

