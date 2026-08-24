import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    color: "#1e1f21"

    Column {
        anchors.fill: parent
        
        // Header with Add Track button
        Rectangle {
            width: parent.width
            height: 40
            color: "#161718"
            
            Button {
                text: "+ Add Track"
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 10
                onClicked: trackListModel.addTrack()
            }
        }
        
        // Track List
        ListView {
            id: trackListView
            width: parent.width
            height: parent.height - 40
            model: trackListModel
            clip: true
            
            delegate: Rectangle {
                width: ListView.view.width
                height: 80
                color: trackColor
                border.color: "#111"
                border.width: 1
                
                Row {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10
                    
                    Column {
                        width: 120
                        spacing: 5
                        Text { text: trackName; color: "white"; font.bold: true }
                        Text { text: volumeDb.toFixed(1) + " dB"; color: "white" }
                    }
                    
                    Button {
                        text: isMuted ? "M" : "M"
                        checkable: true
                        checked: isMuted
                        width: 40
                        onClicked: trackListModel.setData(trackListModel.index(index, 0), checked, 0x0100 + 3) // MuteRole hack
                    }
                    
                    Button {
                        text: "S"
                        checkable: true
                        checked: isSoloed
                        width: 40
                    }
                    
                    Button {
                        text: "+"
                        width: 30
                        onClicked: trackListModel.addMidiClip(index, 0.0, 4.0)
                    }
                }
            }
        }
    }
}
