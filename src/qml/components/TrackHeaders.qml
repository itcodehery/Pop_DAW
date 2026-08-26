import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#252628"
    
    property alias listView: trackListView
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 5
        spacing: 5
        
        ListView {
            id: trackListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4
            clip: true
            
            model: trackListModel
            
            delegate: Rectangle {
                width: parent.width
                height: 80
                color: index === engineController.selectedTrackIndex ? "#3E3F41" : "#252628"
                border.color: index === engineController.selectedTrackIndex ? "#3486F2" : "#1E1F21"
                border.width: 1
                
                MouseArea {
                    anchors.fill: parent
                    onClicked: engineController.selectedTrackIndex = index
                }
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    
                    // Color indicator
                    Rectangle {
                        width: 4
                        height: parent.height - 16
                        color: trackColor || "#555555"
                        radius: 2
                    }
                    
                    Column {
                        Layout.fillWidth: true
                        Layout.leftMargin: 8
                        spacing: 4
                        
                        Text {
                            text: trackName || "Track"
                            color: "#FFFFFF"
                            font.pixelSize: 12
                            font.bold: true
                        }
                        
                        Row {
                            spacing: 4
                            Rectangle {
                                width: 18; height: 18; radius: 3
                                color: isMuted ? "#E06C75" : "#444"
                                Text { text: "M"; color: isMuted ? "#FFF" : "#888"; font.pixelSize: 9; anchors.centerIn: parent }
                                MouseArea { anchors.fill: parent; onClicked: isMuted = !isMuted }
                            }
                            Rectangle {
                                width: 18; height: 18; radius: 3
                                color: isSoloed ? "#E5C07B" : "#444"
                                Text { text: "S"; color: isSoloed ? "#111" : "#888"; font.pixelSize: 9; anchors.centerIn: parent }
                                MouseArea { anchors.fill: parent; onClicked: isSoloed = !isSoloed }
                            }
                            Rectangle {
                                width: 18; height: 18; radius: 3
                                color: "#444"
                                Text { text: "✕"; color: "#888"; font.pixelSize: 9; anchors.centerIn: parent }
                                MouseArea { anchors.fill: parent; onClicked: trackListModel.removeTrack(index) }
                            }
                        }
                    }
                    
                    // Volume meter (bound to levelMeter role)
                    Row {
                        spacing: 2
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        Repeater {
                            model: 2
                            Rectangle {
                                width: 4
                                height: 44
                                color: "#1E1F21"
                                radius: 2
                                Rectangle {
                                    width: parent.width
                                    height: parent.height * (levelMeter || 0)
                                    anchors.bottom: parent.bottom
                                    color: trackColor || "#555"
                                    radius: 2
                                    opacity: 0.8
                                }
                            }
                        }
                    }
                }
            }
        }
        
        Button {
            text: "+ Add Audio"
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            onClicked: engineController.insertAudioFile()
        }
        
        Button {
            text: "+ Add Track"
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            onClicked: trackListModel.addTrack()
        }
    }
}
