#pragma once

#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>
#include "PopLookAndFeel.h"
#include "MixerChannelComponent.h"
#include "ClipGridComponent.h"
#include "SidebarComponent.h"
#include "TopBarComponent.h"
#include "SettingsComponent.h"
#include "PluginHelper.h"

//==============================================================================
class AddPluginButton : public juce::Component
{
public:
    AddPluginButton() {}
    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xff3a3b3d).withAlpha(isMouseOver() ? 0.9f : 0.6f));
        g.drawRoundedRectangle(area.reduced(2.0f), 4.0f, 1.5f);
        g.setColour(juce::Colour(0xffffb380).withAlpha(isMouseOver() ? 1.0f : 0.5f));
        g.setFont(juce::FontOptions(20.0f));
        g.drawText("+", area.withBottom(area.getCentreY()), juce::Justification::centredBottom);
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.drawText("ADD TRACK", area.withTop(area.getCentreY() + 4.0f), juce::Justification::centredTop);
    }
    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit(const juce::MouseEvent&) override { repaint(); }
    void mouseDown(const juce::MouseEvent&) override { if (onClick) onClick(); }
    std::function<void()> onClick;
};

//==============================================================================
class MainComponent : public juce::Component, public juce::FileDragAndDropTarget
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // --- FileDragAndDropTarget ---
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    // Engine/Transport data must be declared before UI components so they outlive them
    tracktion::engine::Engine engine { "Pop DAW" };
    std::unique_ptr<tracktion::engine::Edit> currentEdit;

    PopLookAndFeel customLookAndFeel;
    TopBarComponent topBar;
    SidebarComponent sidebar;
    TimelineComponent timeline;
    std::vector<std::unique_ptr<MixerChannelComponent>> mixerChannels;
    AddPluginButton addPluginBox;

    std::unique_ptr<SettingsComponent> settingsPanel;

    // Pastel color palette for tracks
    std::vector<juce::Colour> trackColors = {
        juce::Colour(0xffff8a8a), // pastel red
        juce::Colour(0xff99ff99), // pastel green
        juce::Colour(0xff99ccff), // pastel blue
        juce::Colour(0xffe8c9ff), // pastel purple
        juce::Colour(0xffffcc99), // pastel orange
        juce::Colour(0xff99ffcc), // pastel teal
        juce::Colour(0xffff99cc), // pastel pink
        juce::Colour(0xffccff99), // pastel lime
    };
    int nextColorIndex = 0;
    juce::Colour getNextTrackColor();

    void addNewTrackWithPlugin(const juce::PluginDescription& desc);
    void addNewTrackWithAudioFile(const juce::File& file);
    void deleteTrack(MixerChannelComponent* channel);
    void addMidiClipToTrack(MixerChannelComponent* channel, double startTime = 0.0);
    void setupTrackCallbacks(MixerChannelComponent* channel, int laneIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
