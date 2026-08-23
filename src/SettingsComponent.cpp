#include "SettingsComponent.h"

//==============================================================================
// AudioSettingsPanel
//==============================================================================
AudioSettingsPanel::AudioSettingsPanel(tracktion::engine::Engine& engine)
{
    auto& dm = engine.getDeviceManager().deviceManager;
    audioDeviceSelector = std::make_unique<juce::AudioDeviceSelectorComponent>(
        dm,
        0, 2,   // min/max input channels
        0, 2,   // min/max output channels
        true,   // show MIDI input options
        true,   // show MIDI output selector
        true,   // show channels as stereo pairs
        false   // hide advanced options
    );

    // Style the selector for dark mode
    audioDeviceSelector->setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff2c2d30));
    audioDeviceSelector->setColour(juce::ListBox::textColourId, juce::Colours::white);

    addAndMakeVisible(audioDeviceSelector.get());
}

AudioSettingsPanel::~AudioSettingsPanel() {}

void AudioSettingsPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff222325));
}

void AudioSettingsPanel::resized()
{
    audioDeviceSelector->setBounds(getLocalBounds().reduced(10));
}

//==============================================================================
// MidiSettingsPanel
//==============================================================================
MidiSettingsPanel::MidiSettingsPanel(tracktion::engine::Engine& eng) : engine(eng)
{
    titleLabel.setText("MIDI Devices", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    // Enumerate MIDI input devices
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    numMidiDevices = std::min((int)midiInputs.size(), 16);

    for (int i = 0; i < numMidiDevices; ++i)
    {
        midiDeviceButtons[i].setButtonText(midiInputs[i].name);
        midiDeviceButtons[i].setColour(juce::ToggleButton::textColourId, juce::Colours::white);
        midiDeviceButtons[i].setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffffb380));
        midiDeviceButtons[i].setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(midiDeviceButtons[i]);
    }
}

void MidiSettingsPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff222325));

    if (numMidiDevices == 0)
    {
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.setFont(juce::FontOptions(14.0f));
        g.drawText("No MIDI devices found.", getLocalBounds().withTrimmedTop(60), juce::Justification::centredTop);
    }
}

void MidiSettingsPanel::resized()
{
    auto area = getLocalBounds().reduced(20);
    titleLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(10);

    for (int i = 0; i < numMidiDevices; ++i)
    {
        midiDeviceButtons[i].setBounds(area.removeFromTop(28));
        area.removeFromTop(4);
    }
}

//==============================================================================
// PluginSettingsPanel
//==============================================================================
PluginSettingsPanel::PluginSettingsPanel(tracktion::engine::Engine& eng)
    : engine(eng)
{
    titleLabel.setText("Plugins", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    auto& pluginManager = engine.getPluginManager();
    juce::File deadMansPedalFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                    .getChildFile("PopDAW").getChildFile("DeadMansPedal");
    
    pluginListComp = std::make_unique<juce::PluginListComponent>(
        pluginManager.pluginFormatManager,
        pluginManager.knownPluginList,
        deadMansPedalFile,
        nullptr, // propertiesToUse
        true // allowAsync
    );
    addAndMakeVisible(pluginListComp.get());
}

void PluginSettingsPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff222325));
}

void PluginSettingsPanel::resized()
{
    auto area = getLocalBounds().reduced(20);
    titleLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(10);
    pluginListComp->setBounds(area);
}
//==============================================================================
// SettingsComponent
//==============================================================================
SettingsComponent::SettingsComponent(tracktion::engine::Engine& eng)
    : engine(eng), audioPanel(eng), midiPanel(eng), pluginPanel(eng)
{
    // Close button
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3a3b3d));
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffb380));
    addAndMakeVisible(closeButton);
    closeButton.onClick = [this]() { if (onClose) onClose(); };

    // Tab buttons
    auto setupTab = [this](juce::TextButton& btn) {
        btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2c2d30));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        btn.setClickingTogglesState(false);
        addAndMakeVisible(btn);
    };

    setupTab(audioTabButton);
    setupTab(midiTabButton);
    setupTab(pluginsTabButton);

    audioTabButton.onClick = [this]() { showTab(Tab::Audio); };
    midiTabButton.onClick = [this]() { showTab(Tab::Midi); };
    pluginsTabButton.onClick = [this]() { showTab(Tab::Plugins); };

    addChildComponent(audioPanel);
    addChildComponent(midiPanel);
    addChildComponent(pluginPanel);

    showTab(Tab::Audio);
}

SettingsComponent::~SettingsComponent() {}

void SettingsComponent::showTab(Tab tab)
{
    currentTab = tab;
    audioPanel.setVisible(tab == Tab::Audio);
    midiPanel.setVisible(tab == Tab::Midi);
    pluginPanel.setVisible(tab == Tab::Plugins);

    // Highlight active tab
    auto activeColor = juce::Colour(0xffffb380);
    auto inactiveColor = juce::Colours::white;

    audioTabButton.setColour(juce::TextButton::textColourOffId, tab == Tab::Audio ? activeColor : inactiveColor);
    midiTabButton.setColour(juce::TextButton::textColourOffId, tab == Tab::Midi ? activeColor : inactiveColor);
    pluginsTabButton.setColour(juce::TextButton::textColourOffId, tab == Tab::Plugins ? activeColor : inactiveColor);

    audioTabButton.setColour(juce::TextButton::buttonColourId, tab == Tab::Audio ? juce::Colour(0xff3a3b3d) : juce::Colour(0xff2c2d30));
    midiTabButton.setColour(juce::TextButton::buttonColourId, tab == Tab::Midi ? juce::Colour(0xff3a3b3d) : juce::Colour(0xff2c2d30));
    pluginsTabButton.setColour(juce::TextButton::buttonColourId, tab == Tab::Plugins ? juce::Colour(0xff3a3b3d) : juce::Colour(0xff2c2d30));

    resized();
}

void SettingsComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1b1d));

    // Settings title
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    g.drawText("Settings", 20, 15, 200, 40, juce::Justification::centredLeft);
}

void SettingsComponent::resized()
{
    auto area = getLocalBounds();

    // Top bar with close button and tabs
    auto topArea = area.removeFromTop(60);
    closeButton.setBounds(topArea.removeFromRight(160).reduced(10));

    // Tab buttons
    auto tabArea = topArea.withTrimmedLeft(160).withTrimmedTop(20);
    audioTabButton.setBounds(tabArea.removeFromLeft(100).reduced(2));
    midiTabButton.setBounds(tabArea.removeFromLeft(100).reduced(2));
    pluginsTabButton.setBounds(tabArea.removeFromLeft(100).reduced(2));

    // Content area
    area.removeFromTop(2);
    auto contentArea = area.reduced(10);

    audioPanel.setBounds(contentArea);
    midiPanel.setBounds(contentArea);
    pluginPanel.setBounds(contentArea);
}
