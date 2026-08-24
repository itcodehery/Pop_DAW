import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    color: "#161718"
    
    Row {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: engineController.timecode
            color: "#888a90"
            font.pixelSize: 24
            font.family: "monospace"
            anchors.verticalCenter: parent.verticalCenter
        }

        Button {
            text: engineController.isPlaying ? "Stop" : "Play"
            onClicked: engineController.togglePlay()
            anchors.verticalCenter: parent.verticalCenter
        }

        Button {
            text: engineController.isRecording ? "Stop Rec" : "Record"
            onClicked: engineController.toggleRecord()
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
