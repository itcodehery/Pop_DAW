#pragma once
#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>

//==============================================================================
// Represents a single audio/MIDI clip block on the timeline
class TimelineClipComponent : public juce::Component
{
public:
    TimelineClipComponent(const juce::String& name, juce::Colour trackColor,
                          double startTime, double lengthSeconds,
                          juce::AudioFormatManager& formatManager,
                          juce::AudioThumbnailCache& thumbCache,
                          const juce::File& audioFile = {},
                          bool isMidi = false);

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    double getStartTime() const { return clipStartTime; }
    void setStartTime(double t) { clipStartTime = t; }
    double getLengthSeconds() const { return clipLength; }
    void setLengthSeconds(double l) { clipLength = l; }
    juce::String getName() const { return clipName; }
    bool getIsMidi() const { return isMidiClip; }
    void setBaseColor(juce::Colour c) { baseColor = c; repaint(); }

    std::function<void(TimelineClipComponent*)> onClipChanged;
    std::function<void(TimelineClipComponent*)> onDeleteClip;

private:
    juce::String clipName;
    juce::Colour baseColor;
    double clipStartTime = 0.0;
    double clipLength = 1.0;
    bool isMidiClip = false;
    juce::File sourceAudioFile;

    std::unique_ptr<juce::AudioThumbnail> thumbnail;

    bool isDragging = false;
    bool isResizing = false;
    double dragStartClipTime = 0.0;
    double dragStartClipLength = 1.0;
    int dragStartMouseX = 0;

    void showContextMenu();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineClipComponent)
};

//==============================================================================
// A single horizontal track row on the timeline
class TimelineTrackLane : public juce::Component
{
public:
    TimelineTrackLane(juce::Colour trackColor,
                      juce::AudioFormatManager& formatManager,
                      juce::AudioThumbnailCache& thumbCache);

    void addAudioClip(const juce::String& name, double startTime, double lengthSeconds, const juce::File& file);
    void addMidiClip(const juce::String& name, double startTime, double lengthSeconds);
    void removeClip(TimelineClipComponent* clip);

    void layoutClips(double pixelsPerSecond, double scrollOffsetSeconds);
    void setTrackColor(juce::Colour c);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

    double getPxPerSec() const { return pxPerSec; }
    double getScrollOffset() const { return scrollOffset; }

    std::function<void(double startTime)> onAddMidiClipAtTime;

private:
    juce::Colour color;
    juce::AudioFormatManager& formatManager;
    juce::AudioThumbnailCache& thumbCache;

    double pxPerSec = 40.0;
    double scrollOffset = 0.0;
    std::vector<std::unique_ptr<TimelineClipComponent>> clips;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineTrackLane)
};

//==============================================================================
// The main timeline component that holds all track lanes and the playhead
class TimelineComponent : public juce::Component, public juce::Timer
{
public:
    TimelineComponent();
    ~TimelineComponent() override;

    void setEdit(tracktion::engine::Edit* editPtr);

    int addTrackLane(juce::Colour trackColor);
    void removeTrackLane(int index);
    TimelineTrackLane* getTrackLane(int index);
    int getNumTrackLanes() const { return (int)trackLanes.size(); }

    void addClipToLane(int laneIndex, const juce::String& name, double startTime, double lengthSeconds, const juce::File& file = {});
    void addMidiClipToLane(int laneIndex, const juce::String& name, double startTime, double lengthSeconds);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void mouseDown(const juce::MouseEvent& e) override;

    double getPixelsPerSecond() const { return pixelsPerSecond; }
    double getScrollOffset() const { return scrollOffsetSeconds; }

private:
    tracktion::engine::Edit* edit = nullptr;
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache { 30 };

    std::vector<std::unique_ptr<TimelineTrackLane>> trackLanes;

    double pixelsPerSecond = 40.0;
    double scrollOffsetSeconds = 0.0;
    double playheadPosition = 0.0; // in seconds

    void drawTimeRuler(juce::Graphics& g);
    void drawPlayhead(juce::Graphics& g);
    void layoutAllLanes();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineComponent)
};
