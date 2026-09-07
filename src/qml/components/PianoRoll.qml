import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#1E1E20"
    
    // Simple mock piano roll
    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // Piano Keys (left)
        Rectangle {
            Layout.preferredWidth: 60
            Layout.fillHeight: true
            color: "#2C2D2F"
            
            ListView {
                anchors.fill: parent
                model: 128 // MIDI notes 0-127
                delegate: Rectangle {
                    width: parent.width
                    height: 12
                    color: [1,3,6,8,10].includes(index % 12) ? "#111111" : "#E8E8E8"
                    border.color: "#555"
                    border.width: 1
                }
            }
        }
        
        // Grid (right)
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1A1A1C"
            
            Text {
                text: "MIDI Piano Roll Interface"
                color: "#555"
                font.pixelSize: 24
                anchors.centerIn: parent
            }
        }
    }
}
