/*
  ==============================================================================

    Pop DAW - Main Application Component
    This is the root component that hosts the entire DAW interface.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>
#include "PopLookAndFeel.h"
#include "MixerChannelComponent.h"
#include "ClipGridComponent.h"

//==============================================================================
/**
    MainComponent - The root UI component for Pop DAW.

    This component initialises the Tracktion Engine and provides
    the foundation for the DAW interface. Currently displays a
    welcome screen with engine status information.
*/
class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // Tracktion Engine instance — the heart of the DAW
    tracktion::engine::Engine engine { "Pop DAW" };
    
    // The currently open project/session
    std::unique_ptr<tracktion::engine::Edit> currentEdit;

    // UI Elements
    PopLookAndFeel customLookAndFeel;
    ClipGridComponent clipGrid;
    std::vector<std::unique_ptr<MixerChannelComponent>> mixerChannels;
    
    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::Label engineInfoLabel;
    juce::TextButton newProjectButton { "New Project" };
    juce::TextButton settingsButton { "Audio Settings" };
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::TextButton importAudioButton { "Import Audio" };

    // Overlay to prevent opening native OS windows (fixes WM scaling issues)
    std::unique_ptr<juce::Component> settingsOverlay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
