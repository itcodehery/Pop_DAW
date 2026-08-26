import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#2C2D2F"
    
    // Top border separator
    Rectangle {
        width: parent.width
        height: 2
        color: "#1E1F21"
        anchors.top: parent.top
    }
    
    ListView {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 2
        orientation: ListView.Horizontal
        clip: true
        
        model: trackListModel
        
        delegate: Rectangle {
            width: 180 // scaled down from 220
            height: parent.height
            color: "#252628"
            border.color: "#1E1F21"
            border.width: 1
            radius: 4
            
            // Background tint based on track color
            Rectangle {
                anchors.fill: parent
                color: trackColor || "#555"
                opacity: 0.1
                radius: 4
            }
            
            // Left half (Track controls)
            Item {
                width: parent.width * 0.55
                height: parent.height
                anchors.left: parent.left
                
                // Round color indicator
                Rectangle {
                    width: 50 // scaled down
                    height: 50
                    radius: 25
                    color: trackColor || "#555"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 20
                    
                    Rectangle {
                        anchors.fill: parent
                        radius: 25
                        color: "transparent"
                        border.color: "#FFFFFF"
                        border.width: 1
                        opacity: 0.2
                    }
                }
                
                Text {
                    text: trackName || "Track"
                    color: "#FFFFFF"
                    font.pixelSize: 12
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 85
                }
                
                // M/S buttons
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 110
                    spacing: 5
                    
                    Rectangle {
                        width: 22
                        height: 18
                        color: isMuted ? "#E06C75" : "#444"
                        radius: 3
                        Text { text: "M"; color: isMuted ? "#FFF" : "#888"; font.pixelSize: 9; anchors.centerIn: parent }
                        MouseArea { anchors.fill: parent; onClicked: isMuted = !isMuted }
                    }
                    Rectangle {
                        width: 22
                        height: 18
                        color: isSoloed ? "#E5C07B" : "#444"
                        radius: 3
                        Text { text: "S"; color: isSoloed ? "#111" : "#888"; font.pixelSize: 9; anchors.centerIn: parent }
                        MouseArea { anchors.fill: parent; onClicked: isSoloed = !isSoloed }
                    }
                }
                
                // Plugin Button
                Rectangle {
                    width: 80
                    height: 24
                    color: "#444"
                    radius: 4
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 15
                    
                    Row {
                        anchors.centerIn: parent
                        spacing: 6
                        Text { text: "🔌"; color: "#FFF"; font.pixelSize: 10 }
                        Text { text: "Plugin"; color: "#FFF"; font.pixelSize: 10 }
                    }
                }
            }
            
            // Divider
            Rectangle {
                width: 1
                height: parent.height - 30
                color: "#000"
                opacity: 0.3
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: parent.width * 0.55
            }
            
            // Right half (Mixer fader)
            Item {
                width: parent.width * 0.45
                height: parent.height
                anchors.right: parent.right
                
                Text {
                    text: "MIXER"
                    color: "#FFFFFF"
                    font.pixelSize: 10
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 10
                }
                
                // Fader slot
                Rectangle {
                    width: 2
                    height: 100 // scaled down
                    color: "#111"
                    anchors.centerIn: parent
                    
                    // Fader handle
                    Rectangle {
                        width: 12
                        height: 12
                        color: trackColor || "#555"
                        radius: 2
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: 100 - ((volumeDb + 60) / 72) * 100 // Map -60 to +12 dB roughly to 0-100 height
                    }
                }
                
                Text {
                    text: volumeDb ? volumeDb.toFixed(1) + " dB" : "0.0 dB"
                    color: "#888"
                    font.pixelSize: 9
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 8
                }
            }
        }
    }
}
