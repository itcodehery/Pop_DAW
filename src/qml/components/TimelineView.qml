import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    color: "#252628"
    clip: true
    
    property real pixelsPerSecond: 40.0
    
    // Time Ruler Placeholder
    Rectangle {
        width: parent.width
        height: 40
        color: "#161718"
        
        Text {
            anchors.centerIn: parent
            text: "Time Ruler"
            color: "#55565a"
        }
    }
    
    // Lanes
    ListView {
        id: timelineView
        width: parent.width
        height: parent.height - 40
        y: 40
        model: trackListModel
        
        delegate: Rectangle {
            width: timelineView.width
            height: 80
            color: index % 2 == 0 ? "#212224" : "#252628"
            
            Repeater {
                model: clipModel
                delegate: Rectangle {
                    x: clipStartTime * root.pixelsPerSecond
                    width: clipLength * root.pixelsPerSecond
                    height: 70
                    y: 5
                    color: clipColor
                    border.color: Qt.lighter(clipColor, 1.2)
                    border.width: 1
                    radius: 4
                    opacity: 0.8
                    
                    Text {
                        anchors.fill: parent
                        anchors.margins: 5
                        text: clipName
                        color: "white"
                        font.bold: true
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}
