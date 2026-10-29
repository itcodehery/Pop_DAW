import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "transparent"
    
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        spacing: 16

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
            Layout.preferredWidth: contentRow.implicitWidth + 36
            Layout.alignment: Qt.AlignVCenter
            color: "#111111"
            radius: 20
            
            Row {
                id: contentRow
                anchors.centerIn: parent
                spacing: 16
                
                // Menus
                Row {
                    spacing: 20
                    anchors.verticalCenter: parent.verticalCenter

                    // FILE dropdown menu
                    Item {
                        width: fileText.implicitWidth
                        height: 24
                        anchors.verticalCenter: parent.verticalCenter
                        Text {
                            id: fileText
                            text: "FILE ▾"
                            color: fileMenu.visible ? "#E5C07B" : "#FFFFFF"
                            font.pixelSize: 11
                            font.bold: true
                            anchors.centerIn: parent
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: fileMenu.open()
                        }
                        Menu {
                            id: fileMenu
                            y: parent.height + 4
                            MenuItem { text: "New Project (Ctrl+N)"; onTriggered: engineController.showNewProjectDialog() }
                            MenuItem { text: "Open Project... (Ctrl+O)"; onTriggered: engineController.openProject() }
                            MenuItem { text: "Save Project (Ctrl+S)"; onTriggered: engineController.saveProject() }
                            MenuItem { text: "Save Project As... (Ctrl+Shift+S)"; onTriggered: engineController.saveProjectAs() }
                            MenuSeparator {}
                            MenuItem { text: "Import Audio File... (Ctrl+I)"; onTriggered: engineController.insertAudioFile() }
                            MenuItem { text: "Export Mixdown (.wav/.flac)... (Ctrl+E)"; onTriggered: engineController.showExportDialog() }
                        }
                    }

                    // EDIT dropdown menu
                    Item {
                        width: editText.implicitWidth
                        height: 24
                        anchors.verticalCenter: parent.verticalCenter
                        Text {
                            id: editText
                            text: "EDIT ▾"
                            color: editMenu.visible ? "#E5C07B" : "#FFFFFF"
                            font.pixelSize: 11
                            font.bold: true
                            anchors.centerIn: parent
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: editMenu.open()
                        }
                        Menu {
                            id: editMenu
                            y: parent.height + 4
                            MenuItem { text: "Undo (Ctrl+Z)"; onTriggered: engineController.undo() }
                            MenuItem { text: "Redo (Ctrl+Y)"; onTriggered: engineController.redo() }
                            MenuSeparator {}
                            MenuItem { text: "Add Audio Track"; onTriggered: trackListModel.addTrack() }
                        }
                    }

                    Text { 
                        text: "INSPECTOR"
                        color: sidebarMode === 0 ? "#E5C07B" : "#FFFFFF"
                        font.pixelSize: 11
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter 
                        MouseArea { anchors.fill: parent; onClicked: sidebarMode = 0 }
                    }

                    Text { 
                        text: "PLUGINS"
                        color: sidebarMode === 2 ? "#E5C07B" : "#FFFFFF"
                        font.pixelSize: 11
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter 
                        MouseArea { anchors.fill: parent; onClicked: sidebarMode = 2 }
                    }

                    Text { 
                        text: "AUDIO"
                        color: sidebarMode === 1 ? "#E5C07B" : "#FFFFFF"
                        font.pixelSize: 11
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter 
                        MouseArea { anchors.fill: parent; onClicked: sidebarMode = 1 }
                    }
                }
                
                Rectangle { width: 1; height: 20; color: "#2C2D2F"; anchors.verticalCenter: parent.verticalCenter }
                
                // Transport Section
                Row {
                    spacing: 12
                    anchors.verticalCenter: parent.verticalCenter
                    
                    // BPM Box (scroll or drag/click to adjust)
                    Rectangle {
                        width: 72
                        height: 26
                        color: "#2C2D2F"
                        radius: 4
                        anchors.verticalCenter: parent.verticalCenter
                        
                        Text {
                            text: engineController.bpm.toFixed(0) + " BPM"
                            color: "#D27C44"
                            font.pixelSize: 11
                            font.bold: true
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            anchors.fill: parent
                            onWheel: (wheel) => {
                                let step = (wheel.modifiers & Qt.ShiftModifier) ? 5 : 1
                                engineController.bpm += (wheel.angleDelta.y > 0 ? step : -step)
                            }
                            onClicked: {
                                let tempos = [90, 100, 110, 120, 128, 140, 160]
                                let cur = Math.round(engineController.bpm)
                                let next = tempos[0]
                                for (let i = 0; i < tempos.length; i++) {
                                    if (tempos[i] > cur) { next = tempos[i]; break; }
                                }
                                engineController.bpm = next
                            }
                        }
                    }
                    
                    // Time Signature Box
                    Rectangle {
                        width: 42
                        height: 26
                        color: "#2C2D2F"
                        radius: 4
                        anchors.verticalCenter: parent.verticalCenter
                        
                        Text {
                            text: engineController.timeSigNumerator + "/" + engineController.timeSigDenominator
                            color: "#D27C44"
                            font.pixelSize: 11
                            font.bold: true
                            anchors.centerIn: parent
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
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
                    
                    // Transport Controls
                    Row {
                        spacing: 10
                        anchors.verticalCenter: parent.verticalCenter
                        
                        Text {
                            text: "⏮"
                            color: "#FFFFFF"
                            font.pixelSize: 15
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea { anchors.fill: parent; onClicked: engineController.rewindToStart() }
                        }
                        Text { 
                            text: engineController.isPlaying ? "⏸" : "▶"
                            color: "#E5C07B"
                            font.pixelSize: 16
                            anchors.verticalCenter: parent.verticalCenter 
                            MouseArea { anchors.fill: parent; onClicked: engineController.togglePlay() }
                        }
                        Text {
                            text: "⏹"
                            color: "#FFFFFF"
                            font.pixelSize: 14
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea { anchors.fill: parent; onClicked: engineController.stop() }
                        }
                        Text {
                            text: "⏭"
                            color: "#FFFFFF"
                            font.pixelSize: 15
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea { anchors.fill: parent; onClicked: engineController.skipForward() }
                        }
                        
                        // Record Dot
                        Rectangle {
                            width: 14
                            height: 14
                            radius: 7
                            color: engineController.isRecording ? "#E06C75" : "#666666"
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea { anchors.fill: parent; onClicked: engineController.toggleRecord() }
                        }

                        // Loop Toggle
                        Rectangle {
                            width: 42
                            height: 22
                            radius: 4
                            color: engineController.isLooping ? "#98E87B" : "#2C2D2F"
                            anchors.verticalCenter: parent.verticalCenter
                            Text {
                                text: "LOOP"
                                color: engineController.isLooping ? "#111111" : "#888888"
                                font.pixelSize: 9
                                font.bold: true
                                anchors.centerIn: parent
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: engineController.isLooping = !engineController.isLooping
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

        Item { Layout.fillWidth: true }

        // Master Level Pill
        Rectangle {
            Layout.preferredWidth: 410
            Layout.preferredHeight: 40
            Layout.alignment: Qt.AlignVCenter
            color: "#1E1E20"
            radius: 4
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8
                
                Rectangle {
                    width: 48
                    height: 24
                    color: engineController.metronomeEnabled ? "#3486F2" : "#2C2D2F"
                    radius: 12
                    Text { text: "CLICK"; color: engineController.metronomeEnabled ? "#FFFFFF" : "#888888"; font.pixelSize: 10; font.bold: true; anchors.centerIn: parent }
                    MouseArea { anchors.fill: parent; onClicked: engineController.metronomeEnabled = !engineController.metronomeEnabled }
                }
                
                Rectangle {
                    width: 48
                    height: 24
                    color: engineController.masterMute ? "#E06C75" : "#2C2D2F"
                    radius: 12
                    Text { text: "MUTE"; color: engineController.masterMute ? "#FFFFFF" : "#888888"; font.pixelSize: 10; font.bold: true; anchors.centerIn: parent }
                    MouseArea { anchors.fill: parent; onClicked: engineController.masterMute = !engineController.masterMute }
                }

                Rectangle {
                    width: 58
                    height: 24
                    color: "#98E87B"
                    radius: 12
                    Text { text: "EXPORT"; color: "#111111"; font.pixelSize: 9; font.bold: true; anchors.centerIn: parent }
                    MouseArea { anchors.fill: parent; onClicked: engineController.showExportDialog() }
                }
                
                // Master Level Meter
                Column {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.leftMargin: 4
                    spacing: 2
                    
                    RowLayout {
                        Text { text: "MASTER"; color: "#888888"; font.pixelSize: 9; font.bold: true }
                        Text {
                            text: engineController.masterVolumeDb <= -90 ? "-INF" : engineController.masterVolumeDb.toFixed(1) + "dB"
                            color: "#FFFFFF"
                            font.pixelSize: 10
                            Layout.leftMargin: 5
                        }
                    }
                    
                    // Meter blocks
                    Row {
                        spacing: 3
                        Repeater {
                            model: 30
                            Rectangle {
                                width: 4
                                height: 4
                                radius: 1
                                color: index < 21 ? "#98E87B" : (index < 26 ? "#E5C07B" : "#E06C75")
                                opacity: (index < (engineController.masterLevel * 30)) ? 1.0 : 0.2
                            }
                        }
                    }
                }
            }
        }
    }
}
