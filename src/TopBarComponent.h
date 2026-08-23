#pragma once
#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>

class TopBarComponent : public juce::Component, public juce::Timer
{
public:
    TopBarComponent();
    ~TopBarComponent() override;

    void setEdit(tracktion::engine::Edit* editPtr);
    
    std::function<void()> onSettingsClicked;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    tracktion::engine::Edit* edit = nullptr;
    bool isPlaying = false;
    bool isRecording = false;

    juce::Label titleLabel;

    // Menus
    juce::TextButton fileMenuButton {"FILE"};
    juce::TextButton viewMenuButton {"VIEW"};
    juce::TextButton editMenuButton {"EDIT"};
    juce::TextButton audioMenuButton {"AUDIO"};
    juce::TextButton helpMenuButton {"HELP"};

    // Transport
    juce::TextButton bpmButton {"120 BPM"};
    juce::ShapeButton prevButton {"prev", juce::Colours::white, juce::Colours::white, juce::Colours::white};
    juce::ShapeButton playButton {"play", juce::Colours::white, juce::Colours::white, juce::Colours::white};
    juce::ShapeButton nextButton {"next", juce::Colours::white, juce::Colours::white, juce::Colours::white};
    juce::ShapeButton recButton {"rec", juce::Colours::red, juce::Colours::red, juce::Colours::red};
    juce::Label timeLabel;

    // Master
    juce::Slider masterVolumeSlider { juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };
    juce::Label masterDbLabel;
    juce::TextButton masterMuteButton {"MUTE"};

    float masterLevelL = 0.0f;
    float masterLevelR = 0.0f;

    void updateTimeDisplay();
    void updateMasterMeter();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopBarComponent)
};
