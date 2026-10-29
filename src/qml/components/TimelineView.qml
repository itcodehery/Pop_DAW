import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PopDAW 1.0

Rectangle {
    id: timelineRoot
    color: "#1E1F21"
    clip: true

    property double pixelsPerSecond: 100.0
    property double scrollOffsetSeconds: 0.0
    property bool snapToGrid: false
    property double gridSnapStep: 0.25 // 250ms when snap is enabled

    function snapTime(sec) {
        if (!snapToGrid) return Math.max(0.0, sec)
        return Math.max(0.0, Math.round(sec / gridSnapStep) * gridSnapStep)
    }

    function timeToX(sec) {
        return (sec - scrollOffsetSeconds) * pixelsPerSecond
    }

    function xToTime(px) {
        return Math.max(0.0, scrollOffsetSeconds + (px / pixelsPerSecond))
    }

    onPixelsPerSecondChanged: {
        rulerCanvas.requestPaint()
        gridCanvas.requestPaint()
    }
    onScrollOffsetSecondsChanged: {
        rulerCanvas.requestPaint()
        gridCanvas.requestPaint()
    }
    onWidthChanged: {
        rulerCanvas.requestPaint()
        gridCanvas.requestPaint()
    }
    onHeightChanged: {
        gridCanvas.requestPaint()
    }

    // Time ruler + zoom/snap toolbar
    Rectangle {
        id: timeRuler
        width: parent.width
        height: 28
        color: "#161719"
        anchors.top: parent.top
        z: 5

        Canvas {
            id: rulerCanvas
            anchors.fill: parent
            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)

                ctx.strokeStyle = "#333438"
                ctx.lineWidth = 1
                ctx.beginPath()
                ctx.moveTo(0, height - 1)
                ctx.lineTo(width, height - 1)
                ctx.stroke()

                // Choose tick interval based on zoom level
                var majorStep = 1.0
                if (timelineRoot.pixelsPerSecond > 250) majorStep = 0.25
                else if (timelineRoot.pixelsPerSecond > 120) majorStep = 0.5
                else if (timelineRoot.pixelsPerSecond > 50) majorStep = 1.0
                else if (timelineRoot.pixelsPerSecond > 20) majorStep = 2.0
                else if (timelineRoot.pixelsPerSecond > 10) majorStep = 5.0
                else majorStep = 10.0

                var subStep = majorStep / 4.0
                var startSec = Math.floor(timelineRoot.scrollOffsetSeconds / subStep) * subStep
                var endSec = timelineRoot.scrollOffsetSeconds + (width / timelineRoot.pixelsPerSecond)

                ctx.font = "10px monospace"
                ctx.fillStyle = "#8E9096"

                for (var t = startSec; t <= endSec; t += subStep) {
                    if (t < 0) continue
                    var x = Math.round((t - timelineRoot.scrollOffsetSeconds) * timelineRoot.pixelsPerSecond)
                    var isMajor = Math.abs(t / majorStep - Math.round(t / majorStep)) < 0.001

                    ctx.strokeStyle = isMajor ? "#666870" : "#323438"
                    ctx.beginPath()
                    ctx.moveTo(x, isMajor ? height - 12 : height - 6)
                    ctx.lineTo(x, height - 1)
                    ctx.stroke()

                    if (isMajor) {
                        var mins = Math.floor(t / 60)
                        var secs = (t % 60)
                        var label = mins > 0
                            ? (mins + ":" + (secs < 10 ? "0" : "") + secs.toFixed(majorStep < 1 ? 2 : 0))
                            : (secs.toFixed(majorStep < 1 ? 2 : 1) + "s")
                        ctx.fillText(label, x + 4, 13)
                    }
                }
            }
        }

        // Loop Region Overlay on Ruler
        Rectangle {
            visible: engineController.isLooping
            x: timelineRoot.timeToX(engineController.loopStartSeconds)
            width: Math.max(4, (engineController.loopEndSeconds - engineController.loopStartSeconds) * timelineRoot.pixelsPerSecond)
            height: parent.height
            color: "#3398E87B"
            border.color: "#98E87B"
            border.width: 1
        }

        // Scrubbing mouse area on ruler
        MouseArea {
            anchors.fill: parent
            anchors.rightMargin: 190
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onPressed: (mouse) => {
                if (mouse.button === Qt.RightButton) {
                    engineController.isLooping = true
                    engineController.loopStartSeconds = timelineRoot.xToTime(mouse.x)
                } else {
                    engineController.setPositionSeconds(timelineRoot.xToTime(mouse.x))
                }
            }
            onPositionChanged: (mouse) => {
                if (pressed) {
                    if (mouse.buttons & Qt.RightButton) {
                        engineController.loopEndSeconds = timelineRoot.xToTime(mouse.x)
                    } else {
                        engineController.setPositionSeconds(timelineRoot.xToTime(mouse.x))
                    }
                }
            }
        }

        // Quick Zoom & Snap controls on top-right of ruler
        Row {
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            spacing: 4

            Rectangle {
                width: 44; height: 20; radius: 3
                color: timelineRoot.snapToGrid ? "#3486F2" : "#252628"
                Text {
                    text: "SNAP"
                    color: timelineRoot.snapToGrid ? "#FFFFFF" : "#888888"
                    font.pixelSize: 9; font.bold: true
                    anchors.centerIn: parent
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: timelineRoot.snapToGrid = !timelineRoot.snapToGrid
                }
            }

            Rectangle {
                width: 22; height: 20; radius: 3; color: "#252628"
                Text { text: "−"; color: "#CCC"; font.pixelSize: 12; anchors.centerIn: parent }
                MouseArea {
                    anchors.fill: parent
                    onClicked: timelineRoot.pixelsPerSecond = Math.max(10.0, timelineRoot.pixelsPerSecond * 0.75)
                }
            }

            Rectangle {
                width: 52; height: 20; radius: 3; color: "#252628"
                Text {
                    text: Math.round(timelineRoot.pixelsPerSecond) + "px/s"
                    color: "#AAA"; font.pixelSize: 9
                    anchors.centerIn: parent
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: timelineRoot.pixelsPerSecond = 100.0
                }
            }

            Rectangle {
                width: 22; height: 20; radius: 3; color: "#252628"
                Text { text: "+"; color: "#CCC"; font.pixelSize: 12; anchors.centerIn: parent }
                MouseArea {
                    anchors.fill: parent
                    onClicked: timelineRoot.pixelsPerSecond = Math.min(800.0, timelineRoot.pixelsPerSecond * 1.33)
                }
            }
        }
    }

    // Background vertical time grid
    Canvas {
        id: gridCanvas
        anchors.top: timeRuler.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        z: 1
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var step = 1.0
            if (timelineRoot.pixelsPerSecond > 250) step = 0.25
            else if (timelineRoot.pixelsPerSecond > 120) step = 0.5
            else if (timelineRoot.pixelsPerSecond > 50) step = 1.0
            else if (timelineRoot.pixelsPerSecond > 20) step = 2.0
            else step = 5.0

            var startSec = Math.floor(timelineRoot.scrollOffsetSeconds / step) * step
            var endSec = timelineRoot.scrollOffsetSeconds + (width / timelineRoot.pixelsPerSecond)

            ctx.strokeStyle = "#28292D"
            ctx.lineWidth = 1
            for (var t = startSec; t <= endSec; t += step) {
                if (t < 0) continue
                var x = Math.round((t - timelineRoot.scrollOffsetSeconds) * timelineRoot.pixelsPerSecond)
                ctx.beginPath()
                ctx.moveTo(x, 0)
                ctx.lineTo(x, height)
                ctx.stroke()
            }
        }
    }

    // Playhead
    Rectangle {
        id: playhead
        visible: x >= 0 && x <= timelineRoot.width
        width: 2
        height: parent.height
        color: "#E06C75"
        x: timelineRoot.timeToX(engineController.positionSeconds)
        z: 20

        Canvas {
            width: 10
            height: 10
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            onPaint: {
                var ctx = getContext("2d")
                ctx.fillStyle = "#E06C75"
                ctx.beginPath()
                ctx.moveTo(0, 0)
                ctx.lineTo(10, 0)
                ctx.lineTo(5, 10)
                ctx.closePath()
                ctx.fill()
            }
        }
    }

    // Wheel handler for Zoom (Ctrl+Wheel) and Horizontal Scroll (Wheel / Shift+Wheel)
    WheelHandler {
        acceptedModifiers: Qt.ControlModifier | Qt.ShiftModifier | Qt.NoModifier
        onWheel: (event) => {
            if (event.modifiers & Qt.ControlModifier) {
                var mouseTime = timelineRoot.xToTime(event.x)
                var factor = event.angleDelta.y > 0 ? 1.2 : (1.0 / 1.2)
                var newPps = Math.max(8.0, Math.min(1000.0, timelineRoot.pixelsPerSecond * factor))
                timelineRoot.pixelsPerSecond = newPps
                timelineRoot.scrollOffsetSeconds = Math.max(0.0, mouseTime - (event.x / newPps))
            } else {
                var delta = (event.angleDelta.x !== 0 ? -event.angleDelta.x : -event.angleDelta.y)
                var secDelta = (delta / 120.0) * (120.0 / timelineRoot.pixelsPerSecond)
                timelineRoot.scrollOffsetSeconds = Math.max(0.0, timelineRoot.scrollOffsetSeconds + secDelta)
            }
        }
    }

    property alias listView: timelineListView

    // Track Lanes
    ListView {
        id: timelineListView
        anchors.top: timeRuler.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: hScrollBar.top
        anchors.margins: 5
        spacing: 2
        z: 2

        model: trackListModel
        clip: true
        interactive: false

        delegate: Rectangle {
            id: trackDelegate
            readonly property int trackIndex: index
            width: timelineListView.width
            height: 68 // Matches TrackHeaders.qml delegate height
            color: index % 2 === 0 ? "#1A1B1E" : "#1E1F22"

            Rectangle {
                width: parent.width
                height: 1
                color: "#2C2D2F"
                anchors.bottom: parent.bottom
            }

            // Click empty track lane to select track, move playhead, or right-click to paste
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: (mouse) => {
                    engineController.selectedTrackIndex = trackDelegate.trackIndex
                    engineController.selectedClipIndex = -1
                    if (mouse.button === Qt.RightButton) {
                        laneContextMenu.clickTime = timelineRoot.snapTime(timelineRoot.xToTime(mouse.x))
                        laneContextMenu.popup()
                    } else {
                        engineController.setPositionSeconds(timelineRoot.snapTime(timelineRoot.xToTime(mouse.x)))
                    }
                }
            }

            Menu {
                id: laneContextMenu
                property double clickTime: 0.0
                MenuItem {
                    text: "Paste Clip Here"
                    onTriggered: trackListModel.pasteClipAt(trackDelegate.trackIndex, laneContextMenu.clickTime)
                }
                MenuItem {
                    text: "Import Audio File..."
                    onTriggered: engineController.insertAudioFile()
                }
            }

            // Drag & drop audio files from OS file manager
            DropArea {
                anchors.fill: parent
                keys: ["text/uri-list", "text/plain"]
                onDropped: (drop) => {
                    var dropTime = timelineRoot.snapTime(timelineRoot.xToTime(drop.x))
                    if (drop.hasUrls && drop.urls.length > 0) {
                        for (let i = 0; i < drop.urls.length; i++) {
                            engineController.insertAudioFileToTrack(trackDelegate.trackIndex, drop.urls[i].toString(), dropTime)
                        }
                    } else if (drop.hasText && drop.text.length > 0) {
                        let lines = drop.text.split("\n")
                        for (let i = 0; i < lines.length; i++) {
                            let trimmed = lines[i].trim()
                            if (trimmed.length > 0) {
                                engineController.insertAudioFileToTrack(trackDelegate.trackIndex, trimmed, dropTime)
                            }
                        }
                    }
                }
            }

            // Clips in this track
            Repeater {
                model: clipModel
                Rectangle {
                    id: clipRect
                    readonly property int clipIdx: index
                    readonly property bool isSelected: (engineController.selectedTrackIndex === trackDelegate.trackIndex &&
                                                        engineController.selectedClipIndex === clipIdx)

                    x: timelineRoot.timeToX(clipStartTime)
                    y: 4
                    width: Math.max(clipLength * timelineRoot.pixelsPerSecond, 4)
                    height: parent.height - 8
                    color: clipColor || "#74A8E8"
                    radius: 4
                    opacity: 0.9
                    border.color: isSelected ? "#FFFFFF" : "#22000000"
                    border.width: isSelected ? 2 : 1
                    clip: true

                    function selectThisClip() {
                        engineController.selectedTrackIndex = trackDelegate.trackIndex
                        engineController.selectedClipIndex = clipRect.clipIdx
                        clipRect.forceActiveFocus()
                    }

                    Keys.onPressed: (event) => {
                        if (event.key === Qt.Key_Delete || event.key === Qt.Key_Backspace) {
                            trackListModel.deleteClip(trackDelegate.trackIndex, clipRect.clipIdx)
                            engineController.selectedClipIndex = -1
                            event.accepted = true
                        } else if (event.key === Qt.Key_S) {
                            trackListModel.splitClip(trackDelegate.trackIndex, clipRect.clipIdx, engineController.positionSeconds)
                            event.accepted = true
                        } else if (event.key === Qt.Key_D && (event.modifiers & Qt.ControlModifier)) {
                            trackListModel.duplicateClip(trackDelegate.trackIndex, clipRect.clipIdx)
                            event.accepted = true
                        } else if (event.key === Qt.Key_C && (event.modifiers & Qt.ControlModifier)) {
                            trackListModel.copyClip(trackDelegate.trackIndex, clipRect.clipIdx)
                            event.accepted = true
                        } else if (event.key === Qt.Key_V && (event.modifiers & Qt.ControlModifier)) {
                            trackListModel.pasteClipAt(trackDelegate.trackIndex, engineController.positionSeconds)
                            event.accepted = true
                        } else if (event.key === Qt.Key_R) {
                            trackListModel.toggleClipReverse(trackDelegate.trackIndex, clipRect.clipIdx)
                            event.accepted = true
                        } else if (event.key === Qt.Key_N) {
                            trackListModel.normalizeClip(trackDelegate.trackIndex, clipRect.clipIdx)
                            event.accepted = true
                        }
                    }

                    // Waveform Rendering
                    WaveformItem {
                        anchors.fill: parent
                        anchors.topMargin: 16
                        anchors.bottomMargin: 2
                        anchors.leftMargin: 2
                        anchors.rightMargin: 2
                        engineController: engineController
                        trackIndex: trackDelegate.trackIndex
                        clipIndex: clipRect.clipIdx
                        clipOffset: model.clipOffset || 0
                        clipLength: model.clipLength || 0
                        clipGainDb: model.clipGainDb || 0
                        fadeIn: model.clipFadeIn || 0
                        fadeOut: model.clipFadeOut || 0
                        isReversed: model.clipIsReversed || false
                    }

                    // Clip Header Bar
                    Rectangle {
                        width: parent.width
                        height: 15
                        color: "#44000000"
                        anchors.top: parent.top

                        Text {
                            text: (clipName || "Audio") + (clipIsReversed ? " [REV]" : "") +
                                  (Math.abs(clipGainDb) > 0.05 ? (" (" + (clipGainDb > 0 ? "+" : "") + clipGainDb.toFixed(1) + "dB)") : "")
                            color: "#FFFFFF"
                            font.pixelSize: 9
                            font.bold: true
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 6
                            anchors.right: parent.right
                            anchors.rightMargin: 6
                            elide: Text.ElideRight
                        }
                    }

                    // Main Body Drag (Move Clip + Context Menu)
                    MouseArea {
                        id: moveArea
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

                        property double dragStartMouseX: 0
                        property double initialStartTime: 0

                        onPressed: (mouse) => {
                            clipRect.selectThisClip()
                            if (mouse.button === Qt.RightButton) {
                                clipContextMenu.popup()
                            } else {
                                dragStartMouseX = mapToItem(timelineListView, mouse.x, 0).x
                                initialStartTime = clipStartTime
                            }
                        }
                        onPositionChanged: (mouse) => {
                            if (pressed && (mouse.buttons & Qt.LeftButton)) {
                                var curX = mapToItem(timelineListView, mouse.x, 0).x
                                var deltaSec = (curX - dragStartMouseX) / timelineRoot.pixelsPerSecond
                                var targetStart = timelineRoot.snapTime(initialStartTime + deltaSec)
                                trackListModel.moveClip(trackDelegate.trackIndex, clipRect.clipIdx, targetStart)
                            }
                        }
                    }

                    // Left Edge Resize Handle (Trim Start & Offset)
                    Rectangle {
                        width: 7
                        height: parent.height
                        anchors.left: parent.left
                        color: leftTrimArea.containsMouse || leftTrimArea.pressed ? "#AAFFFFFF" : "transparent"

                        MouseArea {
                            id: leftTrimArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.SizeHorCursor
                            property double startMouseX: 0
                            property double origStart: 0

                            onPressed: (mouse) => {
                                clipRect.selectThisClip()
                                startMouseX = mapToItem(timelineListView, mouse.x, 0).x
                                origStart = clipStartTime
                            }
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    var curX = mapToItem(timelineListView, mouse.x, 0).x
                                    var deltaSec = (curX - startMouseX) / timelineRoot.pixelsPerSecond
                                    var newStart = timelineRoot.snapTime(origStart + deltaSec)
                                    trackListModel.trimClipLeft(trackDelegate.trackIndex, clipRect.clipIdx, newStart)
                                }
                            }
                        }
                    }

                    // Right Edge Resize Handle (Trim End / Length)
                    Rectangle {
                        width: 7
                        height: parent.height
                        anchors.right: parent.right
                        color: rightTrimArea.containsMouse || rightTrimArea.pressed ? "#AAFFFFFF" : "transparent"

                        MouseArea {
                            id: rightTrimArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.SizeHorCursor
                            property double startMouseX: 0
                            property double origLen: 0

                            onPressed: (mouse) => {
                                clipRect.selectThisClip()
                                startMouseX = mapToItem(timelineListView, mouse.x, 0).x
                                origLen = clipLength
                            }
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    var curX = mapToItem(timelineListView, mouse.x, 0).x
                                    var deltaSec = (curX - startMouseX) / timelineRoot.pixelsPerSecond
                                    var newLen = Math.max(0.05, origLen + deltaSec)
                                    trackListModel.trimClipRight(trackDelegate.trackIndex, clipRect.clipIdx, newLen)
                                }
                            }
                        }
                    }

                    // Top-Left Fade-In Handle
                    Rectangle {
                        width: 8; height: 8; radius: 4
                        x: Math.min(clipRect.width - 10, Math.max(0, (clipFadeIn || 0) * timelineRoot.pixelsPerSecond))
                        y: 2
                        color: "#FFFFFF"
                        border.color: "#111"
                        visible: clipRect.isSelected || fadeInMouse.containsMouse || (clipFadeIn > 0.01)

                        MouseArea {
                            id: fadeInMouse
                            anchors.fill: parent
                            anchors.margins: -4
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    var localX = mapToItem(clipRect, mouse.x, 0).x
                                    var sec = Math.max(0.0, localX / timelineRoot.pixelsPerSecond)
                                    trackListModel.setClipFadeIn(trackDelegate.trackIndex, clipRect.clipIdx, sec)
                                }
                            }
                        }
                    }

                    // Top-Right Fade-Out Handle
                    Rectangle {
                        width: 8; height: 8; radius: 4
                        x: Math.max(2, clipRect.width - 8 - ((clipFadeOut || 0) * timelineRoot.pixelsPerSecond))
                        y: 2
                        color: "#FFFFFF"
                        border.color: "#111"
                        visible: clipRect.isSelected || fadeOutMouse.containsMouse || (clipFadeOut > 0.01)

                        MouseArea {
                            id: fadeOutMouse
                            anchors.fill: parent
                            anchors.margins: -4
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    var localX = mapToItem(clipRect, mouse.x, 0).x
                                    var sec = Math.max(0.0, (clipRect.width - localX) / timelineRoot.pixelsPerSecond)
                                    trackListModel.setClipFadeOut(trackDelegate.trackIndex, clipRect.clipIdx, sec)
                                }
                            }
                        }
                    }

                    // Right-Click Clip Context Menu
                    Menu {
                        id: clipContextMenu
                        MenuItem {
                            text: "Split at Playhead (S)"
                            onTriggered: trackListModel.splitClip(trackDelegate.trackIndex, clipRect.clipIdx, engineController.positionSeconds)
                        }
                        MenuItem {
                            text: "Duplicate Clip (Ctrl+D)"
                            onTriggered: trackListModel.duplicateClip(trackDelegate.trackIndex, clipRect.clipIdx)
                        }
                        MenuItem {
                            text: "Copy Clip (Ctrl+C)"
                            onTriggered: trackListModel.copyClip(trackDelegate.trackIndex, clipRect.clipIdx)
                        }
                        MenuSeparator {}
                        MenuItem {
                            text: clipIsReversed ? "Unreverse Audio (R)" : "Reverse Audio (R)"
                            onTriggered: trackListModel.toggleClipReverse(trackDelegate.trackIndex, clipRect.clipIdx)
                        }
                        MenuItem {
                            text: "Normalize Peak to 0 dB (N)"
                            onTriggered: trackListModel.normalizeClip(trackDelegate.trackIndex, clipRect.clipIdx)
                        }
                        MenuItem {
                            text: "Reset Clip Gain to 0 dB"
                            onTriggered: trackListModel.setClipGainDb(trackDelegate.trackIndex, clipRect.clipIdx, 0.0)
                        }
                        MenuSeparator {}
                        MenuItem {
                            text: "Delete Clip (Del)"
                            onTriggered: {
                                trackListModel.deleteClip(trackDelegate.trackIndex, clipRect.clipIdx)
                                engineController.selectedClipIndex = -1
                            }
                        }
                    }
                }
            }
        }
    }

    // Bottom horizontal navigation bar
    Rectangle {
        id: hScrollBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 12
        color: "#161719"
        z: 10

        Rectangle {
            id: scrollThumb
            height: 8
            anchors.verticalCenter: parent.verticalCenter
            radius: 4
            color: "#3E4046"
            width: Math.max(40, parent.width * 0.15)
            x: Math.min(parent.width - width, (timelineRoot.scrollOffsetSeconds / 300.0) * (parent.width - width))

            MouseArea {
                anchors.fill: parent
                drag.target: scrollThumb
                drag.axis: Drag.XAxis
                drag.minimumX: 0
                drag.maximumX: hScrollBar.width - scrollThumb.width
                onPositionChanged: {
                    if (drag.active) {
                        var ratio = scrollThumb.x / Math.max(1, hScrollBar.width - scrollThumb.width)
                        timelineRoot.scrollOffsetSeconds = ratio * 300.0
                    }
                }
            }
        }
    }
}
