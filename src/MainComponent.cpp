#include "MainComponent.h"
#include <BinaryData.h>

//==============================================================================
MainComponent::MainComponent()
{
    setLookAndFeel(&customLookAndFeel);

    // --- Dummy Mixer Channels ---
    mixerChannels.push_back(std::make_unique<MixerChannelComponent>(nullptr, "PIANO", juce::Colour(0xffff8a8a))); // Pink
    mixerChannels.push_back(std::make_unique<MixerChannelComponent>(nullptr, "GUITAR", juce::Colour(0xff99ff99))); // Green
    mixerChannels.push_back(std::make_unique<MixerChannelComponent>(nullptr, "BASS", juce::Colour(0xff99ccff))); // Blue
    
    for (auto& ch : mixerChannels)
        addAndMakeVisible(ch.get());

    // --- Clip Grid ---
    addAndMakeVisible(clipGrid);
    
    clipGrid.addColumn(juce::Colour(0xffff8a8a)); // Piano
    if (auto* col = clipGrid.getColumn(0)) {
        col->addClip("Piano #1", "Q");
        col->addClip("Piano #2", "W");
    }

    clipGrid.addColumn(juce::Colour(0xff99ff99)); // Guitar
    if (auto* col = clipGrid.getColumn(1)) {
        col->addClip("Guitar #3", "T");
        col->addClip("Guitar #2", "R");
        col->addClip("Guitar #1", "E");
    }

    clipGrid.addColumn(juce::Colour(0xff99ccff)); // Bass
    if (auto* col = clipGrid.getColumn(2)) {
        col->addClip("Bass #1", "Y");
    }

    // --- Sidebar ---
    addAndMakeVisible(sidebar);

    // --- Top Bar ---
    addAndMakeVisible(topBar);
    
    // --- Add Plugin Box ---
    addAndMakeVisible(addPluginBox);
    addPluginBox.onClick = [this]() {
        PluginHelper::showPluginMenu(engine, &addPluginBox, [this](const juce::PluginDescription& desc) {
            juce::Logger::writeToLog("Selected plugin: " + desc.name);
            // We will load the plugin here soon
        });
    };

    setSize(1600, 900);
}

MainComponent::~MainComponent()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // Warm khaki/sand background
    g.fillAll(juce::Colour(0xffcdc4b3));

    // Draw structural lines
    g.setColour(juce::Colour(0xff111111));
    
    float topBarHeight = 70.0f;
    float sidebarWidth = 360.0f;
    float mixerHeight = 250.0f;
    
    // Line under top bar
    g.fillRect(0.0f, topBarHeight, (float)getWidth(), 2.0f);
    
    // Line separating right sidebar
    g.fillRect((float)getWidth() - sidebarWidth, topBarHeight, 2.0f, (float)getHeight() - topBarHeight);
    
    // Line separating mixer
    g.fillRect(0.0f, (float)getHeight() - mixerHeight, (float)getWidth() - sidebarWidth, 2.0f);
    
    // Vertical column separator lines spanning the whole workspace
    int gutterWidth = 30;
    float startX = gutterWidth - 1.0f;
    g.fillRect(startX, topBarHeight, 1.0f, (float)getHeight() - topBarHeight); // gutter line
    
    startX = gutterWidth + 20.0f + 190.0f + 5.0f;
    for (int i=0; i<3; ++i) { // 3 internal separators
        g.fillRect(startX, topBarHeight, 1.0f, (float)getHeight() - topBarHeight);
        startX += 200.0f; // 190 block + 10 spacing
    }
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    
    int topBarHeight = 70;
    int sidebarWidth = 360;
    int mixerHeight = 250;
    
    topBar.setBounds(area.removeFromTop(topBarHeight));
    
    area.removeFromTop(2); // Border
    
    auto sidebarArea = area.removeFromRight(sidebarWidth);
    sidebarArea.removeFromLeft(2); // Border
    sidebar.setBounds(sidebarArea.reduced(20));
    
    auto workspaceArea = area;
    
    auto mixerArea = workspaceArea.removeFromBottom(mixerHeight);
    mixerArea.removeFromTop(2); // Border
    
    // Mixer alignment
    int gutterWidth = 30; // Matches grid
    auto mixerContentArea = mixerArea.withTrimmedLeft(gutterWidth).reduced(20, 10);
    for (auto& ch : mixerChannels)
    {
        ch->setBounds(mixerContentArea.removeFromLeft(190));
        mixerContentArea.removeFromLeft(10); // spacing
    }
    
    // Add Plugin Box
    addPluginBox.setBounds(mixerContentArea.removeFromLeft(190).withTrimmedBottom(20));
    
    clipGrid.setBounds(workspaceArea);
}
