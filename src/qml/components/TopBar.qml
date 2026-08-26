import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "transparent" // Matches main window background
    
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        spacing: 20

        FontLoader { id: logoFont; source: "qrc:/fonts/BBHBartle-Regular.ttf" }

        // Logo
        Text {
            text: "POP"
            color: "#FFFFFF"
            font.pixelSize: 28
            font.family: logoFont.name
            font.bold: true
            font.letterSpacing: 2
            Layout.alignment: Qt.AlignVCenter
        }

        // Unified Menu & Transport Pill
        Rectangle {
            Layout.preferredHeight: 40
            Layout.preferredWidth: contentRow.implicitWidth + 40
            Layout.alignment: Qt.AlignVCenter
            color: "#111111" // Almost black background
            radius: 20
            
            Row {
                id: contentRow
                anchors.centerIn: parent
                spacing: 20
                
                // Menus
                Row {
                    spacing: 25
                    anchors.verticalCenter: parent.verticalCenter

                    Row {
                        spacing: 8
                        Text { text: "FILE"; color: "#FFFFFF"; font.pixelSize: 11; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                        Text { text: "∨"; color: "#888888"; font.pixelSize: 9; anchors.verticalCenter: parent.verticalCenter; anchors.verticalCenterOffset: 1 }
                        MouseArea { anchors.fill: parent; onClicked: engineController.showNewProjectDialog() }
                    }
                    Text { 
                        text: "VIEW"; color: "#FFFFFF"; font.pixelSize: 11; font.bold: true; anchors.verticalCenter: parent.verticalCenter 
                        MouseArea { anchors.fill: parent; onClicked: rightPanelMode = 0 }
                    }
                    Text { 
                        text: "PLUGINS"; color: "#FFFFFF"; font.pixelSize: 11; font.bold: true; anchors.verticalCenter: parent.verticalCenter 
                        MouseArea { anchors.fill: parent; onClicked: rightPanelMode = 2 }
                    }
                    Text { 
                        text: "AUDIO"; color: "#FFFFFF"; font.pixelSize: 11; font.bold: true; anchors.verticalCenter: parent.verticalCenter 
                        MouseArea { anchors.fill: parent; onClicked: rightPanelMode = 1 }
                    }
                    Text { 
                        text: "HELP"; color: "#FFFFFF"; font.pixelSize: 11; font.bold: true; anchors.verticalCenter: parent.verticalCenter 
                    }
                }
                
                // Spacer between menus and transport
                Item { width: 20; height: 1 }
                
                // Transport Section
                Row {
                    spacing: 15
                    anchors.verticalCenter: parent.verticalCenter
                    
                    // BPM Box
                    Rectangle {
                        width: 70
                        height: 26
                        color: "#2C2D2F"
                        radius: 4
                        anchors.verticalCenter: parent.verticalCenter
                        
                        Text {
                            text: engineController.bpm.toFixed(0) + " BPM"
                            color: "#D27C44" // Orange/brown text
                            font.pixelSize: 11
                            font.bold: true
                            anchors.centerIn: parent
                        }
                    }
                    
                    // Time Signature Box
                    Rectangle {
                        width: 45
                        height: 26
                        color: "#2C2D2F"
                        radius: 4
                        anchors.verticalCenter: parent.verticalCenter
                        
                        Text {
                            text: engineController.timeSigNumerator + "/" + engineController.timeSigDenominator
                            color: "#D27C44" // Orange/brown text
                            font.pixelSize: 11
                            font.bold: true
                            anchors.centerIn: parent
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                // Cycle common time signatures
                                if (engineController.timeSigNumerator === 4 && engineController.timeSigDenominator === 4) {
                                    engineController.timeSigNumerator = 3;
                                } else if (engineController.timeSigNumerator === 3 && engineController.timeSigDenominator === 4) {
                                    engineController.timeSigNumerator = 6;
                                    engineController.timeSigDenominator = 8;
                                } else {
                                    engineController.timeSigNumerator = 4;
                                    engineController.timeSigDenominator = 4;
                                }
                            }
                        }
                    }
                    
                    // Controls
                    Row {
                        spacing: 12
                        anchors.verticalCenter: parent.verticalCenter
                        
                        Text { text: "⏮"; color: "#FFFFFF"; font.pixelSize: 16; anchors.verticalCenter: parent.verticalCenter }
                        Text { 
                            text: engineController.isPlaying ? "⏸" : "▶"
                            color: "#E5C07B" // Yellowish play button
                            font.pixelSize: 16
                            anchors.verticalCenter: parent.verticalCenter 
                            
                            MouseArea {
                                anchors.fill: parent
                                onClicked: engineController.togglePlay()
                            }
                        }
                        Text { text: "⏭"; color: "#FFFFFF"; font.pixelSize: 16; anchors.verticalCenter: parent.verticalCenter }
                        
                        // Record Dot
                        Rectangle {
                            width: 14
                            height: 14
                            radius: 7
                            color: engineController.isRecording ? "#E06C75" : "#666666" // Red dot
                            anchors.verticalCenter: parent.verticalCenter
                            
                            MouseArea {
                                anchors.fill: parent
                                onClicked: engineController.toggleRecord()
                            }
                        }
                    }
                    
                    // Timecode
                    Text {
                        text: engineController.timecode
                        color: "#FFFFFF"
                        font.pixelSize: 13
                        font.family: "monospace"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }

        Item { Layout.fillWidth: true } // Spacer

        // Master Level Pill
        Rectangle {
            Layout.preferredWidth: 390
            Layout.preferredHeight: 40
            Layout.alignment: Qt.AlignVCenter
            color: "#1E1E20"
            radius: 4
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 10
                
                Rectangle {
                    width: 50
                    height: 24
                    color: engineController.metronomeEnabled ? "#3486F2" : "#2C2D2F"
                    radius: 12
                    Text { text: "CLICK"; color: engineController.metronomeEnabled ? "#FFFFFF" : "#888888"; font.pixelSize: 10; font.bold: true; anchors.centerIn: parent }
                    MouseArea { anchors.fill: parent; onClicked: engineController.metronomeEnabled = !engineController.metronomeEnabled }
                }
                
                Rectangle {
                    width: 50
                    height: 24
                    color: engineController.masterMute ? "#E06C75" : "#2C2D2F"
                    radius: 12
                    Text { text: "MUTE"; color: engineController.masterMute ? "#FFFFFF" : "#888888"; font.pixelSize: 10; font.bold: true; anchors.centerIn: parent }
                    MouseArea { anchors.fill: parent; onClicked: engineController.masterMute = !engineController.masterMute }
                }
                
                // Master Level Meter
                Column {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.leftMargin: 5
                    spacing: 2
                    
                    RowLayout {
                        Text { text: "MASTER"; color: "#888888"; font.pixelSize: 9; font.bold: true }
                        Text { text: "0.0dB"; color: "#FFFFFF"; font.pixelSize: 10; Layout.leftMargin: 5 }
                        Item { Layout.fillWidth: true }
                        Rectangle { width: 6; height: 6; radius: 3; color: "#E5C07B" } // Yellow dot
                        Text { text: "∨"; color: "#888888"; font.pixelSize: 9; Layout.leftMargin: 5 }
                    }
                    
                    // Meter blocks (horizontal dashes)
                    Row {
                        spacing: 3
                        Repeater {
                            model: 35
                            Rectangle {
                                width: 4
                                height: 3
                                radius: 1
                                color: index < 25 ? "#98E87B" : (index < 30 ? "#E5C07B" : "#E06C75")
                                opacity: (index < (engineController.masterLevel * 35)) ? 1.0 : 0.2
                            }
                        }
                    }
                }
            }
        }
    }
}
