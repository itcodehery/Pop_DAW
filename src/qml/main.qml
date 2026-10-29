import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

ApplicationWindow {
    id: mainWindow
    width: 1440
    height: 900
    visible: true
    title: qsTr("Pop DAW — ") + engineController.projectName
    color: "#323336"
    
    property int sidebarMode: 0 // 0=Inspector/FX, 1=Audio Settings, 2=Plugins
    property int bottomPanelMode: 0 // 0=Mixer, 1=PianoRoll
    property string statusToast: ""

    Connections {
        target: engineController
        function onStatusMessage(message) {
            statusToast = message
            toastTimer.restart()
        }
    }

    Timer {
        id: toastTimer
        interval: 3000
        onTriggered: statusToast = ""
    }

    // Global Keyboard Shortcuts for Audio Editing
    Shortcut {
        sequence: "Space"
        onActivated: engineController.togglePlay()
    }
    Shortcut {
        sequence: "Ctrl+Z"
        onActivated: engineController.undo()
    }
    Shortcut {
        sequences: ["Ctrl+Y", "Ctrl+Shift+Z"]
        onActivated: engineController.redo()
    }
    Shortcut {
        sequence: "Ctrl+S"
        onActivated: engineController.saveProject()
    }
    Shortcut {
        sequence: "Ctrl+Shift+S"
        onActivated: engineController.saveProjectAs()
    }
    Shortcut {
        sequence: "Ctrl+O"
        onActivated: engineController.openProject()
    }
    Shortcut {
        sequence: "Ctrl+N"
        onActivated: engineController.showNewProjectDialog()
    }
    Shortcut {
        sequence: "Ctrl+E"
        onActivated: engineController.showExportDialog()
    }
    Shortcut {
        sequence: "Ctrl+I"
        onActivated: engineController.insertAudioFile()
    }
    Shortcut {
        sequence: "L"
        onActivated: engineController.isLooping = !engineController.isLooping
    }
    Shortcut {
        sequence: "Shift+Tab"
        onActivated: bottomPanelMode = (bottomPanelMode === 0) ? 1 : 0
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        Loader {
            id: topBarLoader
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            source: "qrc:/qml/components/TopBar.qml"
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 2
            
            // Left sidebar
            Loader {
                id: sidebarLoader
                Layout.preferredWidth: 270
                Layout.fillHeight: true
                source: "qrc:/qml/components/Sidebar.qml"
            }

            // Right main section (Timeline & Mixer)
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 2
                
                // Track Headers and Timeline Row
                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 2
                    
                    Loader {
                        id: trackHeadersLoader
                        Layout.preferredWidth: 210
                        Layout.fillHeight: true
                        source: "qrc:/qml/components/TrackHeaders.qml"
                    }
                    
                    Loader {
                        id: timelineLoader
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        source: "qrc:/qml/components/TimelineView.qml"
                    }
                }
                
                Connections {
                    target: trackHeadersLoader.item ? trackHeadersLoader.item.listView : null
                    function onContentYChanged() {
                        if (timelineLoader.item && timelineLoader.item.listView.contentY !== trackHeadersLoader.item.listView.contentY)
                            timelineLoader.item.listView.contentY = trackHeadersLoader.item.listView.contentY
                    }
                }
                Connections {
                    target: timelineLoader.item ? timelineLoader.item.listView : null
                    function onContentYChanged() {
                        if (trackHeadersLoader.item && trackHeadersLoader.item.listView.contentY !== timelineLoader.item.listView.contentY)
                            trackHeadersLoader.item.listView.contentY = timelineLoader.item.listView.contentY
                    }
                }
                
                Loader {
                    id: bottomPanelLoader
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    source: bottomPanelMode === 0 ? "qrc:/qml/components/MixerPanel.qml" : "qrc:/qml/components/PianoRoll.qml"
                }
            }
        }
    }

    // Floating status toast notification
    Rectangle {
        visible: statusToast !== ""
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 16
        height: 32
        width: toastText.implicitWidth + 28
        radius: 6
        color: "#111113"
        border.color: "#98E87B"
        border.width: 1
        z: 100

        Text {
            id: toastText
            anchors.centerIn: parent
            text: statusToast
            color: "#98E87B"
            font.pixelSize: 11
            font.bold: true
        }
    }
}
