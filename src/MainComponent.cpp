/*
  ==============================================================================

    Pop DAW - Main Application Component Implementation

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // --- Title Label ---
    titleLabel.setText("Pop DAW", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(32.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
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
        auto edit = tracktion::engine::CreateEdit::openOrCreate(
            engine,
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                .getChildFile("PopDAW")
                .getChildFile("Untitled.tracktionedit"),
            tracktion::engine::CreateEdit::forEditing,
            nullptr,
            0,
            {}
        );

        if (edit)
        {
            statusLabel.setText("New project created: " + edit->getName(),
                                juce::dontSendNotification);
        }
    };
    addAndMakeVisible(newProjectButton);

    // Set initial window size
    setSize(900, 600);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // Dark background gradient
    g.fillAll(juce::Colour(0xff1a1a2e));

    // Subtle gradient overlay
    juce::ColourGradient gradient(
        juce::Colour(0xff16213e), 0.0f, 0.0f,
        juce::Colour(0xff0f3460), (float) getWidth(), (float) getHeight(),
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
    auto area = getLocalBounds();
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

    // Button centred below
    centreArea.removeFromTop(20);
    auto buttonArea = centreArea.removeFromTop(40);
    newProjectButton.setBounds(buttonArea.withSizeKeepingCentre(200, 36));
}
