#include "MixerChannelComponent.h"

MixerChannelComponent::MixerChannelComponent(tracktion::engine::Track::Ptr t, juce::String name, juce::Colour color)
    : track(t), trackName(name), baseColor(color)
{
    // Left side controls
    muteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff221111));
    muteButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.6f));
    muteButton.setClickingTogglesState(true);
    addAndMakeVisible(muteButton);
    
    soloButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff221111));
    soloButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.6f));
    soloButton.setClickingTogglesState(true);
    addAndMakeVisible(soloButton);
    
    pluginButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff332222));
    addAndMakeVisible(pluginButton);

    // Right side fader
    volumeFader.setSliderStyle(juce::Slider::LinearVertical);
    volumeFader.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeFader.setColour(juce::Slider::thumbColourId, baseColor);
    volumeFader.setColour(juce::Slider::trackColourId, juce::Colours::transparentWhite);
    volumeFader.setColour(juce::Slider::backgroundColourId, juce::Colours::white.withAlpha(0.05f));
    volumeFader.setRange(-60.0, 6.0, 0.1);
    volumeFader.setValue(0.0);
    volumeFader.onValueChange = [this]()
    {
        dbLabel.setText(juce::String(volumeFader.getValue(), 1) + " dB", juce::dontSendNotification);
    };
    addAndMakeVisible(volumeFader);

    dbLabel.setText("0.0 dB", juce::dontSendNotification);
    dbLabel.setJustificationType(juce::Justification::centred);
    dbLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.5f));
    dbLabel.setFont(juce::FontOptions(11.0f));
    addAndMakeVisible(dbLabel);
}

MixerChannelComponent::~MixerChannelComponent() {}

void MixerChannelComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    auto leftArea = area.removeFromLeft(area.getWidth() * 0.55f);
    auto rightArea = area;
    
    // Dark Panel
    g.setColour(juce::Colour(0xff180b0b));
    g.fillRoundedRectangle(leftArea.toFloat(), 4.0f);
    
    // Grey Panel
    g.setColour(juce::Colour(0xff45474a));
    g.fillRoundedRectangle(rightArea.toFloat(), 4.0f);
    
    // Big Circle
    g.setColour(baseColor);
    float padSize = 80.0f;
    juce::Rectangle<float> padArea(leftArea.getCentreX() - padSize/2.0f, 30.0f, padSize, padSize);
    g.fillEllipse(padArea);
    
    // Piano Icon
    g.setColour(juce::Colours::white);
    juce::Rectangle<float> pianoIcon(leftArea.getRight() - 25.0f, 15.0f, 14.0f, 10.0f);
    g.drawRect(pianoIcon, 1.0f);
    g.fillRect(pianoIcon.getX() + 3.0f, pianoIcon.getY(), 2.0f, 6.0f);
    g.fillRect(pianoIcon.getX() + 9.0f, pianoIcon.getY(), 2.0f, 6.0f);
    
    // Track Name
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.drawText(trackName, leftArea.withY(120).withHeight(20), juce::Justification::centred);
    
    // MIXER Title
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("MIXER", rightArea.withY(15).withHeight(20), juce::Justification::centred);
}

void MixerChannelComponent::resized()
{
    auto area = getLocalBounds();
    auto leftArea = area.removeFromLeft(area.getWidth() * 0.55f);
    auto rightArea = area;
    
    auto msArea = leftArea.withY(145).withHeight(22).reduced(25, 0);
    muteButton.setBounds(msArea.removeFromLeft(msArea.getWidth() / 2).reduced(2));
    muteButton.setButtonText("M");
    soloButton.setBounds(msArea.reduced(2));
    soloButton.setButtonText("S");
    
    pluginButton.setBounds(leftArea.withY(180).withHeight(30).reduced(20, 0));
    pluginButton.setButtonText("Plugin");
    
    auto faderArea = rightArea.withY(45).withBottom(getHeight() - 30);
    volumeFader.setBounds(faderArea);
    
    dbLabel.setBounds(rightArea.withTop(getHeight() - 25).withHeight(20));
}
