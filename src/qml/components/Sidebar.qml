import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebarRoot
    color: "#2E2E30"
    
    // Right border separator
    Rectangle {
        width: 2
        height: parent.height
        color: "#1E1F21"
        anchors.right: parent.right
    }

    property var activeClipDetails: ({})

    function refreshClipDetails() {
        if (engineController.selectedTrackIndex >= 0 && engineController.selectedClipIndex >= 0) {
            activeClipDetails = trackListModel.getClipDetails(
                engineController.selectedTrackIndex,
                engineController.selectedClipIndex
            )
        } else {
            activeClipDetails = ({})
        }
    }

    Connections {
        target: engineController
        function onSelectedTrackIndexChanged() { sidebarRoot.refreshClipDetails() }
        function onSelectedClipIndexChanged() { sidebarRoot.refreshClipDetails() }
    }

    Timer {
        interval: 200
        running: engineController.selectedClipIndex >= 0 && sidebarMode === 0
        repeat: true
        onTriggered: sidebarRoot.refreshClipDetails()
    }
    
    StackLayout {
        anchors.fill: parent
        currentIndex: (sidebarMode >= 0 && sidebarMode <= 2) ? sidebarMode : 0
        
        // Mode 0: Clip Audio Editor Inspector & Track Effects Rack
        ColumnLayout {
            anchors.margins: 10
            spacing: 10
            
            // Clip Inspector & Non-Destructive Audio Processing Panel
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 330
                color: "#252628"
                radius: 6
                border.color: "#1E1F21"
                border.width: 1
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8
                    
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "CLIP AUDIO INSPECTOR"
                            color: "#FFFFFF"
                            font.pixelSize: 11
                            font.bold: true
                        }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            width: 8; height: 8; radius: 4
                            color: engineController.selectedClipIndex >= 0 ? "#98E87B" : "#555555"
                        }
                    }

                    Text {
                        text: engineController.selectedClipIndex >= 0
                            ? (sidebarRoot.activeClipDetails.name || "Selected Clip")
                            : "Click any audio clip on the timeline to edit"
                        color: engineController.selectedClipIndex >= 0 ? "#98E87B" : "#888888"
                        font.pixelSize: 10
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    // Clip Gain Slider (-36 dB to +24 dB)
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        enabled: engineController.selectedClipIndex >= 0
                        opacity: enabled ? 1.0 : 0.4

                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "Clip Gain"; color: "#CCCCCC"; font.pixelSize: 10 }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: (sidebarRoot.activeClipDetails.gainDb !== undefined ? sidebarRoot.activeClipDetails.gainDb.toFixed(1) : "0.0") + " dB"
                                color: "#E5C07B"; font.pixelSize: 10; font.bold: true
                            }
                        }
                        Slider {
                            id: gainSlider
                            Layout.fillWidth: true
                            from: -36.0
                            to: 24.0
                            value: sidebarRoot.activeClipDetails.gainDb !== undefined ? sidebarRoot.activeClipDetails.gainDb : 0.0
                            onMoved: {
                                trackListModel.setClipGainDb(engineController.selectedTrackIndex, engineController.selectedClipIndex, value)
                                sidebarRoot.refreshClipDetails()
                            }
                        }
                    }

                    // Fade-In & Fade-Out Sliders
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        enabled: engineController.selectedClipIndex >= 0
                        opacity: enabled ? 1.0 : 0.4

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                text: "Fade In: " + (sidebarRoot.activeClipDetails.fadeIn !== undefined ? sidebarRoot.activeClipDetails.fadeIn.toFixed(2) : "0.00") + "s"
                                color: "#CCCCCC"; font.pixelSize: 9
                            }
                            Slider {
                                Layout.fillWidth: true
                                from: 0.0
                                to: Math.max(0.5, (sidebarRoot.activeClipDetails.length || 2.0) * 0.5)
                                value: sidebarRoot.activeClipDetails.fadeIn || 0.0
                                onMoved: {
                                    trackListModel.setClipFadeIn(engineController.selectedTrackIndex, engineController.selectedClipIndex, value)
                                    sidebarRoot.refreshClipDetails()
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                text: "Fade Out: " + (sidebarRoot.activeClipDetails.fadeOut !== undefined ? sidebarRoot.activeClipDetails.fadeOut.toFixed(2) : "0.00") + "s"
                                color: "#CCCCCC"; font.pixelSize: 9
                            }
                            Slider {
                                Layout.fillWidth: true
                                from: 0.0
                                to: Math.max(0.5, (sidebarRoot.activeClipDetails.length || 2.0) * 0.5)
                                value: sidebarRoot.activeClipDetails.fadeOut || 0.0
                                onMoved: {
                                    trackListModel.setClipFadeOut(engineController.selectedTrackIndex, engineController.selectedClipIndex, value)
                                    sidebarRoot.refreshClipDetails()
                                }
                            }
                        }
                    }

                    // Pitch Shift & Playback Speed
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        enabled: engineController.selectedClipIndex >= 0
                        opacity: enabled ? 1.0 : 0.4

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                text: "Pitch: " + (sidebarRoot.activeClipDetails.pitch !== undefined ? sidebarRoot.activeClipDetails.pitch.toFixed(1) : "0") + " st"
                                color: "#CCCCCC"; font.pixelSize: 9
                            }
                            Slider {
                                Layout.fillWidth: true
                                from: -12.0
                                to: 12.0
                                stepSize: 1.0
                                value: sidebarRoot.activeClipDetails.pitch || 0.0
                                onMoved: {
                                    trackListModel.setClipPitch(engineController.selectedTrackIndex, engineController.selectedClipIndex, value)
                                    sidebarRoot.refreshClipDetails()
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                text: "Speed: " + (sidebarRoot.activeClipDetails.speed !== undefined ? sidebarRoot.activeClipDetails.speed.toFixed(2) : "1.00") + "x"
                                color: "#CCCCCC"; font.pixelSize: 9
                            }
                            Slider {
                                Layout.fillWidth: true
                                from: 0.5
                                to: 2.0
                                stepSize: 0.05
                                value: sidebarRoot.activeClipDetails.speed || 1.0
                                onMoved: {
                                    trackListModel.setClipSpeed(engineController.selectedTrackIndex, engineController.selectedClipIndex, value)
                                    sidebarRoot.refreshClipDetails()
                                }
                            }
                        }
                    }

                    // Quick Audio Editing Action Buttons
                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        rowSpacing: 6
                        columnSpacing: 6
                        enabled: engineController.selectedClipIndex >= 0
                        opacity: enabled ? 1.0 : 0.4

                        Button {
                            text: "Split at Playhead (S)"
                            Layout.fillWidth: true
                            Layout.preferredHeight: 26
                            font.pixelSize: 9
                            onClicked: trackListModel.splitClip(engineController.selectedTrackIndex, engineController.selectedClipIndex, engineController.positionSeconds)
                        }
                        Button {
                            text: "Normalize 0dB (N)"
                            Layout.fillWidth: true
                            Layout.preferredHeight: 26
                            font.pixelSize: 9
                            onClicked: {
                                trackListModel.normalizeClip(engineController.selectedTrackIndex, engineController.selectedClipIndex)
                                sidebarRoot.refreshClipDetails()
                            }
                        }
                        Button {
                            text: (sidebarRoot.activeClipDetails.reversed ? "Unreverse (R)" : "Reverse Audio (R)")
                            Layout.fillWidth: true
                            Layout.preferredHeight: 26
                            font.pixelSize: 9
                            onClicked: {
                                trackListModel.toggleClipReverse(engineController.selectedTrackIndex, engineController.selectedClipIndex)
                                sidebarRoot.refreshClipDetails()
                            }
                        }
                        Button {
                            text: "Duplicate (Ctrl+D)"
                            Layout.fillWidth: true
                            Layout.preferredHeight: 26
                            font.pixelSize: 9
                            onClicked: trackListModel.duplicateClip(engineController.selectedTrackIndex, engineController.selectedClipIndex)
                        }
                    }
                }
            }
            
            // Active Effects Panel for Selected Track
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#252628"
                radius: 6
                border.color: "#1E1F21"
                border.width: 1
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8
                    
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "TRACK EFFECTS CHAIN"
                            color: "#FFFFFF"
                            font.pixelSize: 11
                            font.bold: true
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: "TRACK " + (engineController.selectedTrackIndex + 1)
                            color: "#D27C44"
                            font.pixelSize: 10
                            font.bold: true
                        }
                    }
                    
                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 6
                        clip: true
                        model: engineController.selectedTrackPlugins

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 32
                            color: "#1E1F22"
                            radius: 4
                            border.color: "#323438"
                            border.width: 1
                            
                            Text {
                                text: modelData
                                color: "#FFFFFF"
                                font.pixelSize: 11
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.leftMargin: 8
                                width: parent.width - 80
                                elide: Text.ElideRight
                            }
                            
                            Button {
                                text: "UI"
                                width: 34
                                height: 22
                                font.pixelSize: 9
                                anchors.right: parent.right
                                anchors.rightMargin: 32
                                anchors.verticalCenter: parent.verticalCenter
                                onClicked: engineController.showPluginEditor(engineController.selectedTrackIndex, index)
                            }
                            
                            Rectangle {
                                width: 20
                                height: 20
                                color: "#3A3B3D"
                                radius: 4
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: 6
                                
                                Text {
                                    text: "✕"
                                    color: "#E06C75"
                                    font.pixelSize: 10
                                    anchors.centerIn: parent
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: engineController.removePluginFromTrack(engineController.selectedTrackIndex, index)
                                }
                            }
                        }
                    }
                    
                    // Add Effect Button (switches to Plugin Browser)
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        color: "#1E1F22"
                        border.color: "#3A3B3D"
                        border.width: 1
                        radius: 4
                        
                        Text {
                            text: "+ ADD EFFECT / PLUGIN"
                            color: "#AAAAAA"
                            font.pixelSize: 10
                            font.bold: true
                            anchors.centerIn: parent
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: sidebarMode = 2
                        }
                    }
                }
            }
        }
        
        // Mode 1: Audio I/O & Device Settings
        Rectangle {
            color: "transparent"
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 12
                
                Text { text: "AUDIO I/O SETTINGS"; color: "#FFFFFF"; font.pixelSize: 13; font.bold: true }
                
                Text { text: "Audio Backend (ALSA / JACK / PipeWire):"; color: "#AAAAAA"; font.pixelSize: 11 }
                ComboBox {
                    Layout.fillWidth: true
                    model: engineController.audioDeviceTypes
                    onActivated: {
                        if (currentText !== "") engineController.setAudioDeviceType(currentText)
                    }
                }
                
                Text { text: "Output Device:"; color: "#AAAAAA"; font.pixelSize: 11 }
                ComboBox {
                    Layout.fillWidth: true
                    model: engineController.outputDevices
                    onActivated: {
                        if (currentText !== "") engineController.setOutputDevice(currentText)
                    }
                }

                Text { text: "Recording Input Device:"; color: "#AAAAAA"; font.pixelSize: 11 }
                ComboBox {
                    Layout.fillWidth: true
                    model: engineController.inputDevices
                    onActivated: {
                        if (currentText !== "") engineController.setInputDevice(currentText)
                    }
                }
                
                Item { Layout.fillHeight: true }

                Button {
                    text: "Open Full Audio Setup Dialog..."
                    Layout.fillWidth: true
                    onClicked: engineController.showAudioSettings()
                }
                
                Button {
                    text: "Refresh Audio Devices"
                    Layout.fillWidth: true
                    onClicked: engineController.setAudioDeviceType(engineController.currentDeviceType)
                }
            }
        }
        
        // Mode 2: Plugin Browser
        Rectangle {
            color: "transparent"
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 12
                
                Text { text: "PLUGIN BROWSER (VST3 / LV2)"; color: "#FFFFFF"; font.pixelSize: 13; font.bold: true }
                
                Button {
                    text: "Scan / Manage Plugins..."
                    Layout.fillWidth: true
                    onClicked: engineController.showPluginSettings()
                }
                
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: engineController.availablePlugins
                    spacing: 4
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 36
                        color: "#252628"
                        radius: 4
                        Text {
                            text: modelData
                            color: "#FFFFFF"
                            font.pixelSize: 11
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            width: parent.width - 45
                            elide: Text.ElideRight
                        }
                        Button {
                            text: "+"
                            width: 28
                            height: 26
                            anchors.right: parent.right
                            anchors.rightMargin: 5
                            anchors.verticalCenter: parent.verticalCenter
                            onClicked: engineController.addPluginToTrack(engineController.selectedTrackIndex, modelData)
                        }
                    }
                }
            }
        }
    }
}
