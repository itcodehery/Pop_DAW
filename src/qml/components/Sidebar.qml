import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#2E2E30"
    
    // Left border separator
    Rectangle {
        width: 2
        height: parent.height
        color: "#1E1F21"
        anchors.left: parent.left
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10 // reduced
        spacing: 10
        
        // Active Controller Panel
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 260 // reduced
            color: "#252628"
            radius: 6
            border.color: "#1E1F21"
            border.width: 1
            
            Column {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 15
                
                Text {
                    text: "ACTIVE CONTROLLER"
                    color: "#FFFFFF"
                    font.pixelSize: 12
                    font.bold: true
                }
                
                RowLayout {
                    width: parent.width
                    
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: "#98E87B"
                        Layout.alignment: Qt.AlignVCenter
                    }
                    
                    Text {
                        text: "GUITAR SELECTED"
                        color: "#98E87B"
                        font.pixelSize: 10
                        Layout.alignment: Qt.AlignVCenter
                        Layout.leftMargin: 8
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    Text {
                        text: "Open Plugin"
                        color: "#FFFFFF"
                        font.pixelSize: 10
                        Layout.alignment: Qt.AlignVCenter
                    }
                }
                
                // Knobs Grid
                GridLayout {
                    columns: 2
                    rowSpacing: 20
                    columnSpacing: 30
                    anchors.horizontalCenter: parent.horizontalCenter
                    
                    Repeater {
                        model: ["ATTACK", "RELEASE", "DELAY", "SUSTAIN"]
                        
                        Column {
                            spacing: 10
                            
                            // Mock Knob
                            Item {
                                width: 50 // reduced
                                height: 50
                                anchors.horizontalCenter: parent.horizontalCenter
                                
                                // Knob Track
                                Rectangle {
                                    anchors.fill: parent
                                    radius: 25
                                    color: "transparent"
                                    border.color: "#111"
                                    border.width: 3
                                }
                                
                                // Knob Value (Blue arc)
                                Rectangle {
                                    anchors.fill: parent
                                    radius: 25
                                    color: "transparent"
                                    border.color: "#3486F2"
                                    border.width: 3
                                    clip: true
                                }
                            }
                            
                            Text {
                                text: modelData
                                color: "#FFFFFF"
                                font.pixelSize: 10
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }
                }
            }
        }
        
        // Active Effects Panel
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#252628"
            radius: 6
            border.color: "#1E1F21"
            border.width: 1
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 12
                
                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "ACTIVE EFFECTS"
                        color: "#FFFFFF"
                        font.pixelSize: 12
                        font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: "ENABLED"
                        color: "#D27C44" // Orange
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
                
                // Effects List
                Column {
                    Layout.fillWidth: true
                    spacing: 8
                    
                    Repeater {
                        model: ["Filter Delay", "Limiter", "Resonator", "EQ Eight Band"]
                        
                        Rectangle {
                            width: parent.width
                            height: 32 // reduced
                            color: "#2C2D2F"
                            radius: 4
                            
                            Text {
                                text: modelData
                                color: "#FFFFFF"
                                font.pixelSize: 11
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.leftMargin: 10
                            }
                            
                            Rectangle {
                                width: 20
                                height: 20
                                color: "#3A3B3D"
                                radius: 4
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: 8
                                
                                Text {
                                    text: ">"
                                    color: "#FFF"
                                    font.pixelSize: 10
                                    anchors.centerIn: parent
                                }
                            }
                        }
                    }
                }
                
                Item { Layout.fillHeight: true } // Spacer
                
                // Add Effect Button
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    color: "transparent"
                    border.color: "#3A3B3D"
                    border.width: 2
                    radius: 4
                    
                    Column {
                        anchors.centerIn: parent
                        spacing: 6
                        Text { text: "+"; color: "#555"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "ADD AN EFFECT"; color: "#555"; font.pixelSize: 10; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                }
            }
        }
    }
}
