#include "SidebarComponent.h"

//==============================================================================
ActiveControllerComponent::ActiveControllerComponent()
{
    openPluginButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    openPluginButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(openPluginButton);

    auto setupKnob = [this](juce::Slider& knob, juce::Label& label) {
        knob.setRange(0.0, 1.0, 0.01);
        knob.setValue(0.5);
        knob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffffb380)); // Pastel orange
        addAndMakeVisible(knob);
        
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        label.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        addAndMakeVisible(label);
    };

    setupKnob(knob1, label1);
    setupKnob(knob2, label2);
    setupKnob(knob3, label3);
    setupKnob(knob4, label4);
}

ActiveControllerComponent::~ActiveControllerComponent() {}

void ActiveControllerComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    
    g.setColour(juce::Colour(0xff2c2d30)); // Dark grey background
    g.fillRoundedRectangle(area, 6.0f);

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("ACTIVE CONTROLLER", area.removeFromTop(40).reduced(20, 0).toNearestInt(), juce::Justification::centredLeft);
    
    // Draw horizontal separator
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.fillRect(20.0f, 40.0f, area.getWidth() - 40.0f, 1.0f);
    
    // Track dot and name
    g.setColour(juce::Colour(0xff99ff99)); // Guitar green
    g.fillEllipse(20.0f, 60.0f, 12.0f, 12.0f);
    
    g.setColour(juce::Colours::white);
    g.drawText("GUITAR", 45, 55, 60, 20, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff99ff99));
    g.drawText("SELECTED", 100, 55, 80, 20, juce::Justification::centredLeft);
}

void ActiveControllerComponent::resized()
{
    auto area = getLocalBounds();
    
    openPluginButton.setBounds(area.getWidth() - 100, 55, 80, 20);
    
    auto knobsArea = area.withTop(100).reduced(20, 20);
    int knobSize = 100;
    int labelHeight = 20;
    int colSpacing = (knobsArea.getWidth() - (knobSize * 2)) / 3;
    int rowSpacing = 30;
    
    // Row 1
    int startX = knobsArea.getX() + colSpacing;
    int startY = knobsArea.getY();
    
    knob1.setBounds(startX, startY, knobSize, knobSize);
    label1.setBounds(startX, startY + knobSize, knobSize, labelHeight);
    
    knob2.setBounds(startX + knobSize + colSpacing, startY, knobSize, knobSize);
    label2.setBounds(startX + knobSize + colSpacing, startY + knobSize, knobSize, labelHeight);
    
    // Row 2
    startY += knobSize + labelHeight + rowSpacing;
    
    knob3.setBounds(startX, startY, knobSize, knobSize);
    label3.setBounds(startX, startY + knobSize, knobSize, labelHeight);
    
    knob4.setBounds(startX + knobSize + colSpacing, startY, knobSize, knobSize);
    label4.setBounds(startX + knobSize + colSpacing, startY + knobSize, knobSize, labelHeight);
}

//==============================================================================
ActiveEffectsComponent::ActiveEffectsComponent()
{
    addEffectButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    addEffectButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.6f));
    addAndMakeVisible(addEffectButton);
}

ActiveEffectsComponent::~ActiveEffectsComponent() {}

void ActiveEffectsComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    
    g.setColour(juce::Colour(0xff2c2d30));
    g.fillRoundedRectangle(area, 6.0f);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("ACTIVE EFFECTS", area.removeFromTop(40).reduced(20, 0).toNearestInt(), juce::Justification::centredLeft);
    
    g.setColour(juce::Colour(0xffffb380)); // Pastel Orange ENABLED text
    g.drawText("ENABLED", getLocalBounds().removeFromTop(40).reduced(20, 0), juce::Justification::centredRight);
    
    // Separator
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.fillRect(20.0f, 40.0f, area.getWidth() - 40.0f, 1.0f);
    
    // Draw dummy effect slots
    auto slotsArea = getLocalBounds().withTop(60).reduced(20, 0);
    for (const auto& effectName : dummyEffects)
    {
        auto slot = slotsArea.removeFromTop(36);
        g.setColour(juce::Colour(0xff3a3c40));
        g.fillRoundedRectangle(slot.toFloat(), 4.0f);
        
        g.setColour(juce::Colours::white);
        g.setFont(13.0f);
        g.drawText(effectName, slot.reduced(15, 0), juce::Justification::centredLeft);
        
        // Right arrow icon
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.drawText(">", slot.removeFromRight(30), juce::Justification::centred);
        
        slotsArea.removeFromTop(10);
    }
}

void ActiveEffectsComponent::resized()
{
    auto area = getLocalBounds().reduced(20);
    addEffectButton.setBounds(area.removeFromBottom(80));
}

//==============================================================================
SidebarComponent::SidebarComponent()
{
    addAndMakeVisible(activeController);
    addAndMakeVisible(activeEffects);
}

SidebarComponent::~SidebarComponent() {}

void SidebarComponent::paint(juce::Graphics&) {}

void SidebarComponent::resized()
{
    auto area = getLocalBounds();
    activeController.setBounds(area.removeFromTop(area.getHeight() * 0.55f).reduced(0, 0));
    area.removeFromTop(15);
    activeEffects.setBounds(area);
}
