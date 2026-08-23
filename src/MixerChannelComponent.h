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
    void mouseDown(const juce::MouseEvent& e) override;

    tracktion::engine::Track::Ptr getTrack() const { return track; }
    juce::String getTrackName() const { return trackName; }
    void setTrackName(const juce::String& newName);
    juce::Colour getColor() const { return baseColor; }
    void setColor(juce::Colour newColor);

    std::function<void(MixerChannelComponent*)> onDeleteTrack;
    std::function<void(MixerChannelComponent*)> onAddMidiClip;
    std::function<void(MixerChannelComponent*, juce::Colour)> onColorChanged;

private:
    tracktion::engine::Track::Ptr track;
    juce::String trackName;
    juce::Colour baseColor;
    
    juce::Label trackNameLabel;
    juce::TextButton muteButton { "M" };
    juce::TextButton soloButton { "S" };
    juce::TextButton pluginButton { "Plugin" };
    
    juce::Slider volumeFader { juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };
    juce::Label dbLabel;

    void updateVolumeFromFader();
    void showContextMenu();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerChannelComponent)
};
