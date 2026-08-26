import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PopDAW 1.0

Rectangle {
    color: "#1E1F21"
    
    // Time ruler
    Rectangle {
        id: timeRuler
        width: parent.width
        height: 20 // reduced
        color: "#252628"
        anchors.top: parent.top
        
        // Mock ruler ticks
        Row {
            anchors.fill: parent
            Repeater {
                model: 20
                Item {
                    width: 100
                    height: parent.height
                    Rectangle { width: 1; height: 6; color: "#555"; anchors.bottom: parent.bottom }
                    Text { text: index + 1; color: "#555"; font.pixelSize: 8; anchors.bottom: parent.bottom; anchors.bottomMargin: 8; anchors.left: parent.left; anchors.leftMargin: 2 }
                }
            }
        }
        
        MouseArea {
            anchors.fill: parent
            onPositionChanged: (mouse) => {
                if (pressed) {
                    engineController.setPositionSeconds(Math.max(0, mouse.x / 100.0))
                }
            }
            onClicked: (mouse) => {
                engineController.setPositionSeconds(Math.max(0, mouse.x / 100.0))
            }
        }
    }
    
    // Playhead
    Rectangle {
        id: playhead
        width: 2
        height: parent.height
        color: "#E06C75" // Red playhead
        x: engineController.positionSeconds * 100.0
        z: 10
        
        // Playhead triangle
        Canvas {
            width: 8
            height: 8
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            onPaint: {
                var ctx = getContext("2d");
                ctx.fillStyle = "#E06C75";
                ctx.beginPath();
                ctx.moveTo(0, 0);
                ctx.lineTo(8, 0);
                ctx.lineTo(4, 8);
                ctx.closePath();
                ctx.fill();
            }
        }
        
        MouseArea {
            width: 20
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            cursorShape: Qt.SizeHorCursor
            
            // We use onPositionChanged instead of Drag to keep the property binding active
            onPositionChanged: (mouse) => {
                if (pressed) {
                    var newX = mapToItem(timeRuler, mouse.x, 0).x
                    engineController.setPositionSeconds(Math.max(0, newX / 100.0))
                }
            }
        }
    }
    
    // Grid background
    Grid {
        anchors.top: timeRuler.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        
        Repeater {
            model: 20 // vertical lines
            Rectangle {
                width: 1
                height: 1000
                color: "#2C2D2F"
                x: index * 100
            }
        }
    }
    
    property alias listView: timelineListView
    
    // Track Lanes
    ListView {
        id: timelineListView
        anchors.top: timeRuler.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 5
        spacing: 2 // Match TrackHeaders spacing
        
        model: trackListModel
        clip: true
        interactive: false // Managed by TrackHeaders scroll
        
        delegate: Rectangle {
            id: trackDelegate
            readonly property int trackIndex: index
            width: parent.width
            height: 60 // matches TrackHeaders.qml delegate height
            color: index % 2 === 0 ? "#1A1A1C" : "#1E1F21" // Alternate faint tracking lines
            
            // Grid lines for this track
            Rectangle {
                width: parent.width
                height: 1
                color: "#2C2D2F"
                anchors.bottom: parent.bottom
            }
            
            DropArea {
                anchors.fill: parent
                keys: ["text/uri-list"]
                onDropped: (drop) => {
                    if (drop.hasUrls) {
                        for (let i = 0; i < drop.urls.length; i++) {
                            engineController.insertAudioFileToTrack(trackDelegate.trackIndex, drop.urls[i].toString(), drop.x / 100.0)
                        }
                    }
                }
            }
            
            // Loop over clips in this track
            Repeater {
                model: clipModel
                Rectangle {
                    id: clipRect
                    x: clipStartTime * 100.0
                    y: 10
                    width: Math.max(clipLength * 100.0, 1)
                    height: 40
                    color: clipColor || "#888"
                    radius: 4
                    opacity: 0.8
                    
                    MouseArea {
                        anchors.fill: parent
                        drag.target: clipRect
                        drag.axis: Drag.XAxis
                        onReleased: {
                            trackListModel.moveClip(trackDelegate.trackIndex, index, clipRect.x / 100.0)
                        }
                    }
                    
                    Text { 
                        text: clipName || "Audio Clip"
                        color: "#111"
                        font.pixelSize: 9
                        anchors.margins: 4
                        anchors.top: parent.top
                        anchors.left: parent.left
                        elide: Text.ElideRight
                        width: parent.width - 8
                    }
                    
                    WaveformItem {
                        anchors.fill: parent
                        anchors.margins: 2
                        engineController: engineController
                        trackIndex: trackDelegate.trackIndex
                        clipIndex: index
                    }
                }
            }
        }
    }
}
