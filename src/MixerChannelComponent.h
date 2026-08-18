#pragma once
#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>

class MixerChannelComponent : public juce::Component
{
public:
    MixerChannelComponent(tracktion::engine::Track::Ptr track, juce::String name, juce::Colour baseColor);
    ~MixerChannelComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    tracktion::engine::Track::Ptr track;
    juce::String trackName;
    juce::Colour baseColor;
    
    juce::TextButton muteButton { "M" };
    juce::TextButton soloButton { "S" };
    juce::TextButton pluginButton { "Plugin" };
    
    juce::Slider volumeFader { juce::Slider::LinearVertical, juce::Slider::NoTextBox };
    juce::Label dbLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerChannelComponent)
};
