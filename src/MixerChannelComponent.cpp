#include "MixerChannelComponent.h"

MixerChannelComponent::MixerChannelComponent(tracktion::engine::Track::Ptr t, juce::String name, juce::Colour color)
    : track(t), trackName(name), baseColor(color)
{
    // Left side controls
    muteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff333333));
    muteButton.setClickingTogglesState(true);
    addAndMakeVisible(muteButton);
    
    soloButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff333333));
    soloButton.setClickingTogglesState(true);
    addAndMakeVisible(soloButton);
    
    pluginButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3a3a44));
    addAndMakeVisible(pluginButton);

    // Right side fader
    volumeFader.setColour(juce::Slider::thumbColourId, baseColor);
    volumeFader.setRange(-60.0, 6.0, 0.1);
    volumeFader.setValue(0.0);
    volumeFader.onValueChange = [this]()
    {
        dbLabel.setText(juce::String(volumeFader.getValue(), 1) + " dB", juce::dontSendNotification);
        // TODO: actually map this to the track's volume plugin
    };
    addAndMakeVisible(volumeFader);

    dbLabel.setText("0.0 dB", juce::dontSendNotification);
    dbLabel.setJustificationType(juce::Justification::centred);
    dbLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    dbLabel.setFont(juce::FontOptions(11.0f));
    addAndMakeVisible(dbLabel);
}

MixerChannelComponent::~MixerChannelComponent()
{
}

void MixerChannelComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    
    // Split the background into two halves exactly like the mockup
    auto leftArea = area.removeFromLeft(area.getWidth() / 2);
    auto rightArea = area;
    
    // Left half (Dark panel)
    g.setColour(juce::Colour(0xff1e1212)); // dark brownish black
    g.fillRoundedRectangle(leftArea.toFloat(), 6.0f);
    
    // Right half (Mixer panel)
    g.setColour(juce::Colour(0xff4a4545)); // lighter brown/grey
    g.fillRoundedRectangle(rightArea.toFloat(), 6.0f);
    
    // Draw the big colored trigger pad on the left
    g.setColour(baseColor);
    float padSize = 60.0f;
    juce::Rectangle<float> padArea(leftArea.getCentreX() - padSize/2, 40.0f, padSize, padSize);
    g.fillEllipse(padArea);
    
    // Draw track name
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.drawText(trackName, leftArea.withY(110).withHeight(30), juce::Justification::centred);
    
    // Draw "MIXER" title on the right
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("MIXER", rightArea.withY(15).withHeight(20), juce::Justification::centred);
}

void MixerChannelComponent::resized()
{
    auto area = getLocalBounds();
    auto leftArea = area.removeFromLeft(area.getWidth() / 2);
    auto rightArea = area;
    
    // Position M/S buttons on the left
    auto msArea = leftArea.withY(140).withHeight(25).reduced(20, 0);
    muteButton.setBounds(msArea.removeFromLeft(msArea.getWidth() / 2).reduced(2));
    soloButton.setBounds(msArea.reduced(2));
    
    // Position Plugin button
    pluginButton.setBounds(leftArea.withY(175).withHeight(30).reduced(20, 0));
    
    // Position Fader on the right
    auto faderArea = rightArea.withY(45).withBottom(getHeight() - 30);
    volumeFader.setBounds(faderArea);
    
    // Position dB label
    dbLabel.setBounds(rightArea.withTop(getHeight() - 25).withHeight(20));
}
