import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

ApplicationWindow {
    width: 1440
    height: 900
    visible: true
    title: qsTr("Pop DAW")
    color: "#323336" // Main background color
    
    property int rightPanelMode: 0 // 0=Main, 1=Audio, 2=Plugins
    property int bottomPanelMode: 0 // 0=Mixer, 1=PianoRoll
    
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
            Layout.preferredHeight: 50 // reduced
            source: "qrc:/qml/components/TopBar.qml"
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 2
            
            // Left main section (Timeline & Mixer)
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
                        Layout.preferredWidth: 200 // reduced
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
                    Layout.preferredHeight: 200 // reduced
                    source: bottomPanelMode === 0 ? "qrc:/qml/components/MixerPanel.qml" : "qrc:/qml/components/PianoRoll.qml"
                }
            }
            
            // Right sidebar
            Loader {
                id: sidebarLoader
                Layout.preferredWidth: 260 // reduced
                Layout.fillHeight: true
                source: "qrc:/qml/components/Sidebar.qml"
            }
        }
    }
}
