#pragma once
#include <JuceHeader.h>

class TopBarComponent : public juce::Component
{
public:
    TopBarComponent();
    ~TopBarComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label titleLabel;
    
    // Menu
    std::vector<juce::String> menuItems = {"FILE", "VIEW", "EDIT", "AUDIO", "HELP"};
    
    // Transport
    juce::TextButton bpmButton {"178 BPM"};
    juce::ShapeButton prevButton {"prev", juce::Colours::white, juce::Colours::white, juce::Colours::white};
    juce::ShapeButton playButton {"play", juce::Colours::white, juce::Colours::white, juce::Colours::white};
    juce::ShapeButton nextButton {"next", juce::Colours::white, juce::Colours::white, juce::Colours::white};
    juce::ShapeButton recButton {"rec", juce::Colours::red, juce::Colours::red, juce::Colours::red};
    juce::Label timeLabel;
    
    // Master
    juce::TextButton muteButton {"MUTE"};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopBarComponent)
};
