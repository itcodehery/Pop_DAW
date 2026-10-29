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
        
        // Top header spacer matching TimelineView timeRuler height (24px)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 24
            color: "#1E1F21"
            radius: 3

            Text {
                text: "TRACKS"
                color: "#888888"
                font.pixelSize: 10
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 8
            }
        }

        ListView {
            id: trackListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 2
            clip: true
            
            model: trackListModel
            
            delegate: Rectangle {
                id: trackHeaderDelegate
                width: trackListView.width
                height: 68
                color: index === engineController.selectedTrackIndex ? "#35363A" : "#252628"
                border.color: index === engineController.selectedTrackIndex ? "#3486F2" : "#1E1F21"
                border.width: 1
                radius: 3

                property bool editingName: false
                
                MouseArea {
                    anchors.fill: parent
                    onClicked: engineController.selectedTrackIndex = index
                    onDoubleClicked: {
                        trackHeaderDelegate.editingName = true
                        nameEditInput.text = trackName
                        nameEditInput.forceActiveFocus()
                        nameEditInput.selectAll()
                    }
                }
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 6
                    
                    // Color indicator bar
                    Rectangle {
                        width: 4
                        Layout.fillHeight: true
                        color: trackColor || "#555555"
                        radius: 2
                    }
                    
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 4
                        
                        // Track Name (double click to rename)
                        Item {
                            Layout.fillWidth: true
                            height: 18

                            Text {
                                visible: !trackHeaderDelegate.editingName
                                text: trackName || "Track"
                                color: "#FFFFFF"
                                font.pixelSize: 11
                                font.bold: true
                                elide: Text.ElideRight
                                width: parent.width
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            TextField {
                                id: nameEditInput
                                visible: trackHeaderDelegate.editingName
                                anchors.fill: parent
                                font.pixelSize: 11
                                padding: 2
                                color: "#FFFFFF"
                                background: Rectangle { color: "#1A1B1D"; border.color: "#3486F2"; radius: 2 }
                                onAccepted: {
                                    trackListModel.renameTrack(index, text)
                                    trackHeaderDelegate.editingName = false
                                }
                                onEditingFinished: {
                                    trackHeaderDelegate.editingName = false
                                }
                            }
                        }
                        
                        // Track control buttons: M, S, R (Arm), Volume Slider, Delete
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            // Mute
                            Rectangle {
                                width: 18; height: 18; radius: 3
                                color: isMuted ? "#E06C75" : "#3A3B3E"
                                Text { text: "M"; color: isMuted ? "#FFF" : "#AAA"; font.pixelSize: 9; font.bold: true; anchors.centerIn: parent }
                                MouseArea { anchors.fill: parent; onClicked: trackListModel.toggleTrackMute(index) }
                            }
                            // Solo
                            Rectangle {
                                width: 18; height: 18; radius: 3
                                color: isSoloed ? "#E5C07B" : "#3A3B3E"
                                Text { text: "S"; color: isSoloed ? "#111" : "#AAA"; font.pixelSize: 9; font.bold: true; anchors.centerIn: parent }
                                MouseArea { anchors.fill: parent; onClicked: trackListModel.toggleTrackSolo(index) }
                            }
                            // Record Arm
                            Rectangle {
                                width: 18; height: 18; radius: 3
                                color: isArmed ? "#E06C75" : "#3A3B3E"
                                border.color: isArmed ? "#FF9999" : "transparent"
                                Text { text: "R"; color: isArmed ? "#FFF" : "#AAA"; font.pixelSize: 9; font.bold: true; anchors.centerIn: parent }
                                MouseArea { anchors.fill: parent; onClicked: trackListModel.toggleTrackArm(index) }
                            }
                            // Delete Track
                            Rectangle {
                                width: 18; height: 18; radius: 3
                                color: "#3A3B3E"
                                Text { text: "✕"; color: "#888"; font.pixelSize: 9; anchors.centerIn: parent }
                                MouseArea { anchors.fill: parent; onClicked: trackListModel.removeTrack(index) }
                            }

                            Item { Layout.fillWidth: true }

                            Text {
                                text: (volumeDb !== undefined ? volumeDb.toFixed(1) : "0.0") + "dB"
                                color: "#999999"
                                font.pixelSize: 9
                            }
                        }

                        // Compact horizontal volume bar
                        Rectangle {
                            Layout.fillWidth: true
                            height: 6
                            radius: 3
                            color: "#18191B"

                            Rectangle {
                                width: Math.max(4, Math.min(parent.width, ((volumeDb + 60.0) / 72.0) * parent.width))
                                height: parent.height
                                radius: 3
                                color: trackColor || "#74A8E8"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onPressed: (mouse) => {
                                    engineController.selectedTrackIndex = index
                                    let ratio = Math.max(0.0, Math.min(1.0, mouse.x / width))
                                    trackListModel.setTrackVolume(index, ratio * 72.0 - 60.0)
                                }
                                onPositionChanged: (mouse) => {
                                    if (pressed) {
                                        let ratio = Math.max(0.0, Math.min(1.0, mouse.x / width))
                                        trackListModel.setTrackVolume(index, ratio * 72.0 - 60.0)
                                    }
                                }
                                onDoubleClicked: trackListModel.setTrackVolume(index, 0.0)
                            }
                        }
                    }
                    
                    // Live Stereo Level Meter
                    Row {
                        spacing: 2
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        Repeater {
                            model: 2
                            Rectangle {
                                width: 4
                                height: 50
                                color: "#18191B"
                                radius: 2
                                Rectangle {
                                    width: parent.width
                                    height: parent.height * Math.min(1.0, (levelMeter || 0))
                                    anchors.bottom: parent.bottom
                                    color: (levelMeter > 0.9) ? "#E06C75" : (trackColor || "#98E87B")
                                    radius: 2
                                }
                            }
                        }
                    }
                }
            }
        }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            Button {
                text: "+ Import Audio"
                Layout.fillWidth: true
                Layout.preferredHeight: 28
                onClicked: engineController.insertAudioFile()
            }
            
            Button {
                text: "+ Track"
                Layout.preferredWidth: 70
                Layout.preferredHeight: 28
                onClicked: trackListModel.addTrack()
            }
        }
    }
}
