/*
  ==============================================================================

    Pop DAW - Main Application Component Implementation

  ==============================================================================
*/

#include "MainComponent.h"
#include <BinaryData.h>

//==============================================================================
class SettingsOverlay : public juce::Component
{
public:
    SettingsOverlay(tracktion::engine::Engine& engine)
    {
        selector.reset(new juce::AudioDeviceSelectorComponent(engine.getDeviceManager().deviceManager, 
                                                              0, 256, 0, 256, true, true, true, false));
        addAndMakeVisible(selector.get());
        
        closeButton.onClick = [this] { setVisible(false); };
        addAndMakeVisible(closeButton);
    }

    void paint(juce::Graphics& g) override
    {
        // Dark semi-transparent background to dim the main UI
        g.fillAll(juce::Colours::black.withAlpha(0.85f));
        
        // Solid background for the dialog area
        auto area = getLocalBounds().reduced(40);
        g.setColour(juce::Colour(0xff222233));
        g.fillRoundedRectangle(area.toFloat(), 8.0f);
        
        g.setColour(juce::Colours::white);
        g.setFont(20.0f);
        g.drawText("Audio Settings", area.removeFromTop(40).toNearestInt(), juce::Justification::centred);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(40);
        area.removeFromTop(40); // Leave space for title
        closeButton.setBounds(area.removeFromBottom(50).reduced(150, 10));
        selector->setBounds(area.reduced(10));
    }

private:
    std::unique_ptr<juce::AudioDeviceSelectorComponent> selector;
    juce::TextButton closeButton { "Close" };
};

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

    // --- Title Label ---
    auto logoTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::BBHBartleRegular_ttf, BinaryData::BBHBartleRegular_ttfSize);
    juce::Font logoFont(juce::FontOptions(logoTypeface).withHeight(48.0f));
    
    titleLabel.setText("POP", juce::dontSendNotification);
    titleLabel.setFont(logoFont);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff222222)); // Dark color for contrast
    addAndMakeVisible(titleLabel);

    // --- Status Label ---
    juce::String statusText = "Engine Status: ";
    statusText += "Active";
    statusText += "  |  Sample Rate: " + juce::String(engine.getDeviceManager().getSampleRate()) + " Hz";
    statusText += "  |  Buffer Size: " + juce::String(engine.getDeviceManager().getBlockSize()) + " samples";

    statusLabel.setText(statusText, juce::dontSendNotification);
    statusLabel.setFont(juce::FontOptions(14.0f));
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(statusLabel);

    // --- Engine Info Label ---
    juce::String infoText;
    infoText += "Powered by Tracktion Engine\n";
    infoText += "Audio Backend: ";

    auto& dm = engine.getDeviceManager();
    if (auto* currentDevice = dm.deviceManager.getCurrentAudioDevice())
        infoText += currentDevice->getTypeName();
    else
        infoText += "No audio device";

    engineInfoLabel.setText(infoText, juce::dontSendNotification);
    engineInfoLabel.setFont(juce::FontOptions(13.0f));
    engineInfoLabel.setJustificationType(juce::Justification::centred);
    engineInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xff888888));
    addAndMakeVisible(engineInfoLabel);

    // --- New Project Button ---
    newProjectButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a90d9));
    newProjectButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    newProjectButton.onClick = [this]()
    {
        // Create a new Edit (Tracktion Engine's equivalent of a project/session)
        auto editFile = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                            .getChildFile("PopDAW")
                            .getChildFile("Untitled.tracktionedit");
        
        currentEdit = tracktion::engine::createEmptyEdit(engine, editFile);

        if (currentEdit)
        {
            statusLabel.setText("New project created: " + currentEdit->getName(),
                                juce::dontSendNotification);
        }
    };
    addAndMakeVisible(newProjectButton);

    // --- Audio Settings Overlay ---
    settingsOverlay = std::make_unique<SettingsOverlay>(engine);
    addChildComponent(settingsOverlay.get());

    // --- Audio Settings Button ---
    settingsButton.onClick = [this]()
    {
        settingsOverlay->setVisible(true);
        settingsOverlay->setBounds(getLocalBounds());
    };
    addAndMakeVisible(settingsButton);

    // --- Transport Buttons ---
    playButton.onClick = [this]() { if (currentEdit) currentEdit->getTransport().play(false); };
    stopButton.onClick = [this]() { if (currentEdit) currentEdit->getTransport().stop(false, false); };
    importAudioButton.onClick = [this]() { /* To be implemented */ };
    
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(importAudioButton);

    // Set initial window size
    setSize(900, 600);
}

MainComponent::~MainComponent()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // Warm khaki/sand background
    g.fillAll(juce::Colour(0xffc2bba8));

    // Subtle gradient overlay
    juce::ColourGradient gradient(
        juce::Colour(0xffd2cbb8), 0.0f, 0.0f,
        juce::Colour(0xffb2ab98), (float) getWidth(), (float) getHeight(),
        false
    );
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());

    // Version text in bottom-right
    g.setColour(juce::Colour(0xff555555));
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("v" + juce::String(JUCE_APPLICATION_VERSION_STRING),
               getLocalBounds().removeFromBottom(25).removeFromRight(100),
               juce::Justification::centredRight);
}

void MainComponent::resized()
{
    if (settingsOverlay != nullptr)
        settingsOverlay->setBounds(getLocalBounds());

    auto area = getLocalBounds();
    
    // Position mixer channels at the bottom
    auto mixerArea = area.removeFromBottom(250).reduced(20, 10);
    for (auto& ch : mixerChannels)
    {
        ch->setBounds(mixerArea.removeFromLeft(170));
        mixerArea.removeFromLeft(10); // Spacing
    }
    
    // Position the clip grid above the mixer
    clipGrid.setBounds(area.reduced(20, 10));

    auto centreArea = area.reduced(40);

    // Layout from the centre of the window
    auto topSection = centreArea.removeFromTop(centreArea.getHeight() / 2);

    // Title in the upper-centre area
    titleLabel.setBounds(topSection.removeFromBottom(50));

    // Status below the title
    statusLabel.setBounds(centreArea.removeFromTop(30));

    // Engine info below status
    centreArea.removeFromTop(10);
    engineInfoLabel.setBounds(centreArea.removeFromTop(50));

    // Buttons layout
    centreArea.removeFromTop(20);
    
    auto buttonRow1 = centreArea.removeFromTop(40);
    newProjectButton.setBounds(buttonRow1.removeFromLeft(140));
    buttonRow1.removeFromLeft(20); // spacing
    settingsButton.setBounds(buttonRow1.removeFromLeft(140));
    
    centreArea.removeFromTop(10);
    auto buttonRow2 = centreArea.removeFromTop(40);
    playButton.setBounds(buttonRow2.removeFromLeft(80));
    buttonRow2.removeFromLeft(10);
    stopButton.setBounds(buttonRow2.removeFromLeft(80));
    buttonRow2.removeFromLeft(20);
    importAudioButton.setBounds(buttonRow2.removeFromLeft(140));
}
