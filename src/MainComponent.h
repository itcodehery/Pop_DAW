#pragma once

#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>
#include "PopLookAndFeel.h"
#include "MixerChannelComponent.h"
#include "ClipGridComponent.h"
#include "SidebarComponent.h"
#include "TopBarComponent.h"
#include "PluginHelper.h"

//==============================================================================
class AddPluginButton : public juce::Component
{
public:
    AddPluginButton() {}
    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds().toFloat();
        g.setColour(juce::Colours::black.withAlpha(isMouseOver() ? 0.8f : 0.6f));
        g.drawRoundedRectangle(area, 4.0f, 2.0f);
        g.setColour(juce::Colour(0xff111111));
        g.setFont(juce::FontOptions(24.0f));
        g.drawText("+", area.withBottom(area.getCentreY()), juce::Justification::centredBottom);
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.drawText("ADD A PLUGIN", area.withTop(area.getCentreY() + 10.0f), juce::Justification::centredTop);
    }
    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit(const juce::MouseEvent&) override { repaint(); }
    void mouseDown(const juce::MouseEvent&) override { if (onClick) onClick(); }
    std::function<void()> onClick;
};

//==============================================================================
class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    tracktion::engine::Engine engine { "Pop DAW" };

    PopLookAndFeel customLookAndFeel;
    TopBarComponent topBar;
    SidebarComponent sidebar;
    ClipGridComponent clipGrid;
    std::vector<std::unique_ptr<MixerChannelComponent>> mixerChannels;
    AddPluginButton addPluginBox;
    
    // Engine/Transport data
    std::unique_ptr<tracktion::engine::Edit> currentEdit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
