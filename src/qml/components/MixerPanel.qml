import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#222326"
    
    // Top border separator
    Rectangle {
        width: parent.width
        height: 2
        color: "#161719"
        anchors.top: parent.top
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        // Horizontal list of all Track Channel Strips
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 6
            orientation: ListView.Horizontal
            clip: true
            
            model: trackListModel
            
            delegate: Rectangle {
                width: 145
                height: ListView.view.height
                color: index === engineController.selectedTrackIndex ? "#2C2E33" : "#252629"
                border.color: index === engineController.selectedTrackIndex ? (trackColor || "#3486F2") : "#1A1B1E"
                border.width: index === engineController.selectedTrackIndex ? 2 : 1
                radius: 5

                MouseArea {
                    anchors.fill: parent
                    onClicked: engineController.selectedTrackIndex = index
                }
                
                // Top color strip
                Rectangle {
                    width: parent.width
                    height: 4
                    radius: 2
                    color: trackColor || "#74A8E8"
                    anchors.top: parent.top
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4

                    // Track Name
                    Text {
                        text: trackName || ("Track " + (index + 1))
                        color: "#FFFFFF"
                        font.pixelSize: 11
                        font.bold: true
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    // Pan Slider (-1.0 Left to +1.0 Right)
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        Text { text: "L"; color: "#777"; font.pixelSize: 8 }
                        Slider {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 16
                            from: -1.0
                            to: 1.0
                            value: pan || 0.0
                            onMoved: trackListModel.setTrackPan(index, value)
                            onDoubleClicked: trackListModel.setTrackPan(index, 0.0)
                        }
                        Text { text: "R"; color: "#777"; font.pixelSize: 8 }
                    }

                    // Middle section: M/S/R buttons on left, Fader + Meter on right
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 8

                        Column {
                            spacing: 4
                            Layout.alignment: Qt.AlignVCenter

                            Rectangle {
                                width: 26; height: 20; radius: 3
                                color: isMuted ? "#E06C75" : "#3A3B3E"
                                Text { text: "M"; color: isMuted ? "#FFF" : "#AAA"; font.pixelSize: 9; font.bold: true; anchors.centerIn: parent }
                                MouseArea { anchors.fill: parent; onClicked: trackListModel.toggleTrackMute(index) }
                            }
                            Rectangle {
                                width: 26; height: 20; radius: 3
                                color: isSoloed ? "#E5C07B" : "#3A3B3E"
                                Text { text: "S"; color: isSoloed ? "#111" : "#AAA"; font.pixelSize: 9; font.bold: true; anchors.centerIn: parent }
                                MouseArea { anchors.fill: parent; onClicked: trackListModel.toggleTrackSolo(index) }
                            }
                            Rectangle {
                                width: 26; height: 20; radius: 3
                                color: isArmed ? "#E06C75" : "#3A3B3E"
                                Text { text: "R"; color: isArmed ? "#FFF" : "#AAA"; font.pixelSize: 9; font.bold: true; anchors.centerIn: parent }
                                MouseArea { anchors.fill: parent; onClicked: trackListModel.toggleTrackArm(index) }
                            }
                        }

                        Item { Layout.fillWidth: true }

                        // Vertical Volume Fader
                        Item {
                            width: 24
                            Layout.fillHeight: true

                            Rectangle {
                                width: 4
                                height: parent.height - 8
                                radius: 2
                                color: "#151618"
                                anchors.centerIn: parent

                                Rectangle {
                                    id: faderHandle
                                    width: 16
                                    height: 10
                                    radius: 2
                                    color: trackColor || "#74A8E8"
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    y: Math.max(0, Math.min(parent.height - height,
                                        (1.0 - ((volumeDb + 60.0) / 72.0)) * (parent.height - height)))
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    anchors.margins: -8
                                    onPressed: (mouse) => {
                                        engineController.selectedTrackIndex = index
                                        var norm = 1.0 - Math.max(0.0, Math.min(1.0, mouse.y / height))
                                        trackListModel.setTrackVolume(index, norm * 72.0 - 60.0)
                                    }
                                    onPositionChanged: (mouse) => {
                                        if (pressed) {
                                            var norm = 1.0 - Math.max(0.0, Math.min(1.0, mouse.y / height))
                                            trackListModel.setTrackVolume(index, norm * 72.0 - 60.0)
                                        }
                                    }
                                    onDoubleClicked: trackListModel.setTrackVolume(index, 0.0)
                                }
                            }
                        }

                        // Vertical Level Meter
                        Rectangle {
                            width: 6
                            Layout.fillHeight: true
                            Layout.topMargin: 4
                            Layout.bottomMargin: 4
                            color: "#151618"
                            radius: 3

                            Rectangle {
                                width: parent.width
                                height: parent.height * Math.min(1.0, (levelMeter || 0))
                                anchors.bottom: parent.bottom
                                radius: 3
                                color: (levelMeter > 0.9) ? "#E06C75" : (trackColor || "#98E87B")
                            }
                        }
                    }

                    // Bottom dB readout
                    Text {
                        text: (volumeDb !== undefined ? volumeDb.toFixed(1) : "0.0") + " dB"
                        color: "#999999"
                        font.pixelSize: 9
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }

        // Dedicated Master Strip on Right
        Rectangle {
            Layout.preferredWidth: 130
            Layout.fillHeight: true
            color: "#1C1D20"
            border.color: "#E5C07B"
            border.width: 1
            radius: 5

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                Text {
                    text: "MASTER BUS"
                    color: "#E5C07B"
                    font.pixelSize: 11
                    font.bold: true
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                Slider {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    orientation: Qt.Vertical
                    from: -60.0
                    to: 6.0
                    value: engineController.masterVolumeDb
                    onMoved: engineController.masterVolumeDb = value
                }

                Text {
                    text: engineController.masterVolumeDb <= -90 ? "-INF" : (engineController.masterVolumeDb.toFixed(1) + " dB")
                    color: "#FFFFFF"
                    font.pixelSize: 10
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }
}
