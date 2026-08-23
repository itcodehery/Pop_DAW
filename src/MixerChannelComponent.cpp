#include "MixerChannelComponent.h"
#include "PluginHelper.h"

MixerChannelComponent::MixerChannelComponent(tracktion::engine::Track::Ptr t, juce::String name, juce::Colour color)
    : track(t), trackName(name), baseColor(color)
{
    // Track Name Label (Editable on double click)
    trackNameLabel.setText(trackName, juce::dontSendNotification);
    trackNameLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    trackNameLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    trackNameLabel.setEditable(false, true, false);
    trackNameLabel.onTextChange = [this]() {
        setTrackName(trackNameLabel.getText());
    };
    addAndMakeVisible(trackNameLabel);

    // Left side controls
    muteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff221111));
    muteButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.6f));
    muteButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffff5555));
    muteButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    muteButton.setClickingTogglesState(true);
    muteButton.onClick = [this]() {
        if (track)
            track->setMute(muteButton.getToggleState());
    };
    addAndMakeVisible(muteButton);
    
    soloButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff221111));
    soloButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.6f));
    soloButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffffbb33));
    soloButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    soloButton.setClickingTogglesState(true);
    soloButton.onClick = [this]() {
        if (track)
            track->setSolo(soloButton.getToggleState());
    };
    addAndMakeVisible(soloButton);
    
    pluginButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff332222));
    pluginButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.8f));
    pluginButton.onClick = [this]() {
        if (!track) return;
        PluginHelper::showPluginMenu(track->edit.engine, &pluginButton, [this](const juce::PluginDescription& desc) {
            juce::Logger::writeToLog("Loading plugin: " + desc.name + " onto track " + trackName);
            auto newPlugin = track->edit.engine.getPluginManager().createNewPlugin(track->edit, tracktion::engine::ExternalPlugin::xmlTypeName, desc);
            if (newPlugin != nullptr) {
                track->pluginList.insertPlugin(newPlugin, 0, nullptr);
            }
        });
    };
    addAndMakeVisible(pluginButton);

    // Right side fader
    volumeFader.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeFader.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeFader.setColour(juce::Slider::thumbColourId, baseColor);
    volumeFader.setColour(juce::Slider::trackColourId, juce::Colours::transparentWhite);
    volumeFader.setColour(juce::Slider::backgroundColourId, juce::Colours::white.withAlpha(0.05f));
    volumeFader.setRange(-60.0, 6.0, 0.1);
    volumeFader.setValue(0.0);
    volumeFader.onValueChange = [this]() {
        updateVolumeFromFader();
    };
    addAndMakeVisible(volumeFader);

    dbLabel.setText("0.0 dB", juce::dontSendNotification);
    dbLabel.setJustificationType(juce::Justification::centred);
    dbLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.5f));
    dbLabel.setFont(juce::FontOptions(10.0f));
    addAndMakeVisible(dbLabel);

    // Initial state sync
    if (track)
    {
        muteButton.setToggleState(track->isMuted(false), juce::dontSendNotification);
        soloButton.setToggleState(track->isSolo(false), juce::dontSendNotification);
        if (auto* at = dynamic_cast<tracktion::engine::AudioTrack*>(track.get()))
        {
            if (auto* vp = at->getVolumePlugin())
                volumeFader.setValue(vp->getVolumeDb(), juce::dontSendNotification);
        }
    }
}

MixerChannelComponent::~MixerChannelComponent() {}

void MixerChannelComponent::setTrackName(const juce::String& newName)
{
    trackName = newName.trim();
    if (trackName.isEmpty())
        trackName = "Track";
    trackNameLabel.setText(trackName, juce::dontSendNotification);
    if (track)
        track->setName(trackName);
    repaint();
}

void MixerChannelComponent::setColor(juce::Colour newColor)
{
    baseColor = newColor;
    volumeFader.setColour(juce::Slider::thumbColourId, baseColor);
    repaint();
}

void MixerChannelComponent::updateVolumeFromFader()
{
    float db = (float)volumeFader.getValue();
    dbLabel.setText(juce::String(db, 1) + " dB", juce::dontSendNotification);
    if (auto* at = dynamic_cast<tracktion::engine::AudioTrack*>(track.get()))
    {
        if (auto* vp = at->getVolumePlugin())
            vp->setVolumeDb(db);
    }
}

void MixerChannelComponent::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        showContextMenu();
    }
}

void MixerChannelComponent::showContextMenu()
{
    juce::PopupMenu menu;
    menu.addItem(1, "Rename Track...");
    menu.addItem(2, "Add MIDI Clip");
    menu.addSeparator();

    juce::PopupMenu colorSubMenu;
    std::vector<std::pair<juce::String, juce::Colour>> colors = {
        { "Red", juce::Colour(0xffff8a8a) },
        { "Green", juce::Colour(0xff99ff99) },
        { "Blue", juce::Colour(0xff99ccff) },
        { "Purple", juce::Colour(0xffe8c9ff) },
        { "Orange", juce::Colour(0xffffcc99) },
        { "Teal", juce::Colour(0xff99ffcc) },
        { "Pink", juce::Colour(0xffff99cc) },
        { "Lime", juce::Colour(0xffccff99) }
    };
    for (size_t i = 0; i < colors.size(); ++i)
    {
        colorSubMenu.addItem((int)(10 + i), colors[i].first);
    }
    menu.addSubMenu("Track Color", colorSubMenu);

    menu.addSeparator();
    menu.addItem(99, "Delete Track");

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
        [this, colors](int result) {
            if (result == 1)
            {
                trackNameLabel.showEditor();
            }
            else if (result == 2)
            {
                if (onAddMidiClip) onAddMidiClip(this);
            }
            else if (result >= 10 && result < (int)(10 + colors.size()))
            {
                auto chosenColor = colors[(size_t)(result - 10)].second;
                setColor(chosenColor);
                if (onColorChanged) onColorChanged(this, chosenColor);
            }
            else if (result == 99)
            {
                if (onDeleteTrack) onDeleteTrack(this);
            }
        });
}

void MixerChannelComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();

    // Background
    g.setColour(juce::Colour(0xff2c2d30));
    g.fillRoundedRectangle(area.toFloat(), 4.0f);

    // Colored circle
    g.setColour(baseColor);
    g.fillEllipse(62.0f, 10.0f, 14.0f, 14.0f);
}

void MixerChannelComponent::resized()
{
    // Compact layout for 60px height
    // Row 1: M/S buttons + track name label + dB label
    // Row 2: Plugin button + volume fader

    muteButton.setBounds(8, 8, 22, 18);
    soloButton.setBounds(33, 8, 22, 18);

    trackNameLabel.setBounds(80, 8, getWidth() - 145, 18);
    dbLabel.setBounds(getWidth() - 62, 8, 56, 18);

    pluginButton.setBounds(8, 34, 52, 18);
    volumeFader.setBounds(65, 34, getWidth() - 75, 18);
}
