import QtQuick
import QtQuick.Controls
import QtQuick.Window

Window {
    width: 1600
    height: 900
    visible: true
    title: qsTr("Pop DAW")
    color: "#222325"

    Column {
        anchors.fill: parent
        
        Loader {
            id: topBarLoader
            width: parent.width
            height: 70
            source: "qrc:/qml/components/TopBar.qml"
        }

        Rectangle {
            width: parent.width
            height: parent.height - topBarLoader.height
            color: "transparent"
            
            Row {
                anchors.fill: parent
                
                Loader {
                    id: trackHeadersLoader
                    width: 300
                    height: parent.height
                    source: "qrc:/qml/components/TrackHeaders.qml"
                }
                
                Loader {
                    id: timelineLoader
                    width: parent.width - trackHeadersLoader.width
                    height: parent.height
                    source: "qrc:/qml/components/TimelineView.qml"
                }
            }
        }
    }
}
