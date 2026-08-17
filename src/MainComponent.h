/*
  ==============================================================================

    Pop DAW - Main Application Component
    This is the root component that hosts the entire DAW interface.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>

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

    // UI Elements
    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::Label engineInfoLabel;
    juce::TextButton newProjectButton { "New Project" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
