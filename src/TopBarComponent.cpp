#include "TopBarComponent.h"
#include <BinaryData.h>

TopBarComponent::TopBarComponent()
{
    auto logoTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::BBHBartleRegular_ttf, BinaryData::BBHBartleRegular_ttfSize);
    juce::Font logoFont(juce::FontOptions(logoTypeface).withHeight(60.0f));
    
    titleLabel.setText("POP", juce::dontSendNotification);
    titleLabel.setFont(logoFont);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff111111));
    addAndMakeVisible(titleLabel);
    
    bpmButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a1e1e));
    bpmButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd48148)); // Orangeish
    addAndMakeVisible(bpmButton);
    
    // Setup transport shapes
    juce::Path playPath;
    playPath.addTriangle(0, 0, 0, 14, 12, 7);
    playButton.setShape(playPath, false, true, false);
    addAndMakeVisible(playButton);
    
    juce::Path prevPath;
    prevPath.addTriangle(10, 0, 10, 14, 0, 7);
    prevPath.addTriangle(20, 0, 20, 14, 10, 7);
    prevButton.setShape(prevPath, false, true, false);
    addAndMakeVisible(prevButton);
    
    juce::Path nextPath;
    nextPath.addTriangle(0, 0, 0, 14, 10, 7);
    nextPath.addTriangle(10, 0, 10, 14, 20, 7);
    nextButton.setShape(nextPath, false, true, false);
    addAndMakeVisible(nextButton);
    
    juce::Path recPath;
    recPath.addEllipse(0, 0, 14, 14);
    recButton.setShape(recPath, false, true, false);
    addAndMakeVisible(recButton);
    
    timeLabel.setText("1:05:248", juce::dontSendNotification);
    timeLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    timeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    timeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(timeLabel);
}

TopBarComponent::~TopBarComponent() {}

void TopBarComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    
    // Menu items
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    float x = 160.0f; // Shifted right to accommodate bigger logo
    for (const auto& item : menuItems) {
        g.drawText(item, juce::Rectangle<float>(x, 0, 70, area.getHeight()), juce::Justification::centred);
        x += 70.0f;
    }
    
    // Transport Background
    float cx = area.getWidth() * 0.5f;
    float transportWidth = 350.0f;
    float transportHeight = 44.0f;
    juce::Rectangle<float> transportArea(cx - transportWidth*0.5f, (area.getHeight() - transportHeight)*0.5f, transportWidth, transportHeight);
    
    g.setColour(juce::Colour(0xff180d0d)); // Dark brown
    g.fillRoundedRectangle(transportArea, 10.0f);
    
    // Master Area Background
    float masterWidth = 400.0f;
    juce::Rectangle<float> masterArea(area.getWidth() - masterWidth - 20.0f, transportArea.getY(), masterWidth, transportHeight);
    g.fillRoundedRectangle(masterArea, 10.0f);
    
    g.setColour(juce::Colours::white);
    g.setFont(11.0f);
    g.drawText("MASTER", masterArea.withWidth(60).translated(10, 0).withHeight(20), juce::Justification::centredLeft);
    g.drawText("0.0Hz", masterArea.withWidth(60).translated(70, 0).withHeight(20), juce::Justification::centredLeft);
    
    // Draw master meters (dummy)
    g.setColour(juce::Colour(0xff66cc66)); // Green meter
    float meterY = masterArea.getY() + 25.0f;
    float meterW = masterArea.getWidth() - 20.0f;
    for (int i=0; i<30; ++i) {
        if (i > 25) g.setColour(juce::Colours::red);
        else if (i > 20) g.setColour(juce::Colours::yellow);
        g.fillRect(masterArea.getX() + 10.0f + i * (meterW/30.0f), meterY, (meterW/30.0f) - 2.0f, 6.0f);
    }
}

void TopBarComponent::resized()
{
    auto area = getLocalBounds();
    titleLabel.setBounds(20, 0, 200, area.getHeight());
    
    float cx = area.getWidth() * 0.5f;
    float transportWidth = 350.0f;
    float transportHeight = 44.0f;
    auto transportArea = juce::Rectangle<int>(cx - transportWidth*0.5f, (area.getHeight() - transportHeight)*0.5f, transportWidth, transportHeight);
    
    bpmButton.setBounds(transportArea.removeFromLeft(80).reduced(2));
    
    transportArea.removeFromLeft(20);
    prevButton.setBounds(transportArea.removeFromLeft(20).withSizeKeepingCentre(16, 12));
    transportArea.removeFromLeft(15);
    playButton.setBounds(transportArea.removeFromLeft(20).withSizeKeepingCentre(12, 12));
    transportArea.removeFromLeft(15);
    nextButton.setBounds(transportArea.removeFromLeft(20).withSizeKeepingCentre(16, 12));
    transportArea.removeFromLeft(20);
    
    recButton.setBounds(transportArea.removeFromLeft(20).withSizeKeepingCentre(12, 12));
    timeLabel.setBounds(transportArea);
}
