#include "MainComponent.h"
#include <BinaryData.h>

//==============================================================================
MainComponent::MainComponent()
{
    setLookAndFeel(&customLookAndFeel);
    engine.getDeviceManager().initialise(0, 2);
    engine.getPluginManager().initialise();

    juce::File editFile = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("PopDAW_Session.tracktionedit");
    currentEdit = tracktion::engine::createEmptyEdit(engine, editFile);
    currentEdit->getTransport().ensureContextAllocated();

    // --- Timeline ---
    addAndMakeVisible(timeline);
    timeline.setEdit(currentEdit.get());

    // --- Sidebar ---
    addAndMakeVisible(sidebar);

    // --- Top Bar ---
    addAndMakeVisible(topBar);
    topBar.setEdit(currentEdit.get());

    // --- Add Plugin/Track Box ---
    addAndMakeVisible(addPluginBox);
    addPluginBox.onClick = [this]() {
        PluginHelper::showPluginMenu(engine, &addPluginBox, [this](const juce::PluginDescription& desc) {
            addNewTrackWithPlugin(desc);
        });
    };

    // --- Settings Panel ---
    settingsPanel = std::make_unique<SettingsComponent>(engine);
    addChildComponent(settingsPanel.get());
    settingsPanel->onClose = [this]() {
        settingsPanel->setVisible(false);
        timeline.setVisible(true);
        sidebar.setVisible(true);
        for (auto& ch : mixerChannels) ch->setVisible(true);
        addPluginBox.setVisible(true);
    };

    topBar.onSettingsClicked = [this]() {
        timeline.setVisible(false);
        sidebar.setVisible(false);
        for (auto& ch : mixerChannels) ch->setVisible(false);
        addPluginBox.setVisible(false);
        settingsPanel->setVisible(true);
        settingsPanel->setBounds(getLocalBounds().withTrimmedTop(70));
    };

    setSize(1600, 900);
}

MainComponent::~MainComponent()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
juce::Colour MainComponent::getNextTrackColor()
{
    auto color = trackColors[(size_t)nextColorIndex % trackColors.size()];
    nextColorIndex++;
    return color;
}

void MainComponent::setupTrackCallbacks(MixerChannelComponent* channel, int laneIndex)
{
    if (channel == nullptr) return;

    channel->onDeleteTrack = [this](MixerChannelComponent* ch) {
        deleteTrack(ch);
    };

    channel->onAddMidiClip = [this](MixerChannelComponent* ch) {
        addMidiClipToTrack(ch, 0.0);
    };

    channel->onColorChanged = [this, laneIndex](MixerChannelComponent*, juce::Colour newColor) {
        if (auto* lane = timeline.getTrackLane(laneIndex))
            lane->setTrackColor(newColor);
    };

    if (auto* lane = timeline.getTrackLane(laneIndex))
    {
        lane->onAddMidiClipAtTime = [this, channel](double startTime) {
            addMidiClipToTrack(channel, startTime);
        };
    }
}

void MainComponent::addNewTrackWithPlugin(const juce::PluginDescription& desc)
{
    juce::Logger::writeToLog("Selected plugin: " + desc.name);

    // Create a new track in the engine
    currentEdit->ensureNumberOfAudioTracks(tracktion::engine::getAudioTracks(*currentEdit).size() + 1);
    auto updatedTracks = tracktion::engine::getAudioTracks(*currentEdit);
    auto newTrack = updatedTracks.getLast();

    // Add plugin to the track
    auto newPlugin = engine.getPluginManager().createNewPlugin(*currentEdit, tracktion::engine::ExternalPlugin::xmlTypeName, desc);
    if (newPlugin != nullptr)
        newTrack->pluginList.insertPlugin(newPlugin, 0, nullptr);

    // Add UI components
    auto color = getNextTrackColor();
    mixerChannels.push_back(std::make_unique<MixerChannelComponent>(newTrack, desc.name.substring(0, 12).toUpperCase(), color));
    addAndMakeVisible(mixerChannels.back().get());

    int laneIndex = timeline.addTrackLane(color);
    setupTrackCallbacks(mixerChannels.back().get(), laneIndex);

    resized();
    repaint();
}

void MainComponent::addNewTrackWithAudioFile(const juce::File& file)
{
    // Create a new track in the engine
    currentEdit->ensureNumberOfAudioTracks(tracktion::engine::getAudioTracks(*currentEdit).size() + 1);
    auto updatedTracks = tracktion::engine::getAudioTracks(*currentEdit);
    auto targetTrack = updatedTracks.getLast();

    // Insert the audio file as a WaveAudioClip at position 0
    tracktion::engine::AudioFile audioFile(engine, file);
    double fileLengthSeconds = audioFile.getLength();

    if (fileLengthSeconds > 0.0)
    {
        tracktion::TimeRange clipRange(tracktion::TimePosition::fromSeconds(0.0),
                                       tracktion::TimePosition::fromSeconds(fileLengthSeconds));
        tracktion::engine::ClipPosition clipPos { clipRange, {} };

        auto newClip = targetTrack->insertWaveClip(
            file.getFileNameWithoutExtension(), file,
            clipPos, false);

        if (newClip != nullptr)
            juce::Logger::writeToLog("Inserted clip: " + newClip->getName() + " (" + juce::String(fileLengthSeconds, 2) + "s)");
    }

    // Add UI components
    auto color = getNextTrackColor();
    mixerChannels.push_back(std::make_unique<MixerChannelComponent>(targetTrack, file.getFileNameWithoutExtension().substring(0, 12).toUpperCase(), color));
    addAndMakeVisible(mixerChannels.back().get());

    int laneIndex = timeline.addTrackLane(color);
    setupTrackCallbacks(mixerChannels.back().get(), laneIndex);

    if (fileLengthSeconds > 0.0)
        timeline.addClipToLane(laneIndex, file.getFileNameWithoutExtension(), 0.0, fileLengthSeconds, file);

    resized();
    repaint();
}

void MainComponent::deleteTrack(MixerChannelComponent* channel)
{
    if (channel == nullptr) return;

    for (size_t i = 0; i < mixerChannels.size(); ++i)
    {
        if (mixerChannels[i].get() == channel)
        {
            if (auto t = channel->getTrack())
            {
                if (currentEdit != nullptr)
                    currentEdit->deleteTrack(t.get());
            }

            timeline.removeTrackLane((int)i);
            mixerChannels.erase(mixerChannels.begin() + (int)i);
            break;
        }
    }

    // Re-bind lane indices for remaining tracks
    for (size_t i = 0; i < mixerChannels.size(); ++i)
    {
        setupTrackCallbacks(mixerChannels[i].get(), (int)i);
    }

    resized();
    repaint();
}

void MainComponent::addMidiClipToTrack(MixerChannelComponent* channel, double startTime)
{
    if (channel == nullptr || currentEdit == nullptr) return;

    for (size_t i = 0; i < mixerChannels.size(); ++i)
    {
        if (mixerChannels[i].get() == channel)
        {
            if (auto* at = dynamic_cast<tracktion::engine::AudioTrack*>(channel->getTrack().get()))
            {
                tracktion::TimeRange range(tracktion::TimePosition::fromSeconds(startTime),
                                           tracktion::TimePosition::fromSeconds(startTime + 4.0));
                at->insertMIDIClip("MIDI Clip", range, nullptr);
            }

            timeline.addMidiClipToLane((int)i, "MIDI Clip", startTime, 4.0);
            break;
        }
    }
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // Dark mode background
    g.fillAll(juce::Colour(0xff222325));

    // Draw structural lines
    g.setColour(juce::Colour(0xff121213));

    float topBarHeight = 70.0f;
    float sidebarWidth = 360.0f;
    float trackHeaderWidth = 220.0f;

    // Line under top bar
    g.fillRect(0.0f, topBarHeight, (float)getWidth(), 2.0f);

    // Line separating right sidebar
    g.fillRect((float)getWidth() - sidebarWidth, topBarHeight, 2.0f, (float)getHeight() - topBarHeight);

    // Line separating track headers from timeline
    g.fillRect(trackHeaderWidth, topBarHeight, 2.0f, (float)getHeight() - topBarHeight);

    // Horizontal row separator lines in the track header area
    float startY = topBarHeight + 90.0f;
    int numRows = std::max(4, (int)mixerChannels.size() + 1);
    for (int i = 0; i < numRows; ++i)
    {
        g.fillRect(0.0f, startY, trackHeaderWidth, 1.0f);
        startY += 72.0f;
    }
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    int topBarHeight = 70;
    int sidebarWidth = 360;
    int trackHeaderWidth = 220;

    topBar.setBounds(area.removeFromTop(topBarHeight));

    area.removeFromTop(2); // Border

    auto sidebarArea = area.removeFromRight(sidebarWidth);
    sidebarArea.removeFromLeft(2); // Border
    sidebar.setBounds(sidebarArea.reduced(20));

    // Track Headers alignment
    float y = area.getY() + 38.0f;
    for (auto& ch : mixerChannels) {
        ch->setBounds(10, (int)y, trackHeaderWidth - 20, 60);
        y += 72.0f;
    }

    addPluginBox.setBounds(10, (int)y, trackHeaderWidth - 20, 60);

    // Timeline
    auto workspaceArea = area;
    workspaceArea.removeFromLeft(trackHeaderWidth);
    timeline.setBounds(workspaceArea);

    if (settingsPanel != nullptr)
        settingsPanel->setBounds(getLocalBounds().withTrimmedTop(topBarHeight));
}

//==============================================================================
bool MainComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto file : files) {
        juce::File f(file);
        if (f.hasFileExtension("wav") || f.hasFileExtension("flac") || f.hasFileExtension("mp3") || f.hasFileExtension("ogg") || f.hasFileExtension("mid"))
            return true;
    }
    return false;
}

void MainComponent::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/)
{
    for (auto file : files) {
        juce::File f(file);
        if (f.existsAsFile()) {
            addNewTrackWithAudioFile(f);
        }
    }
}
