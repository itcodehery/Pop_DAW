#include "EngineController.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <tracktion_engine/tracktion_engine.h>
#include <cmath>

EngineController::EngineController(tracktion::engine::Engine& eng, tracktion::engine::Edit* ed, QObject* parent)
    : QObject(parent), engine(eng), edit(ed)
{
    connect(&updateTimer, &QTimer::timeout, this, &EngineController::updateState);
    updateTimer.start(30); // ~30 FPS UI updates
}

EngineController::~EngineController() = default;

bool EngineController::isPlaying() const
{
    return edit ? edit->getTransport().isPlaying() : false;
}

bool EngineController::isRecording() const
{
    return edit ? edit->getTransport().isRecording() : false;
}

QString EngineController::timecode() const
{
    if (!edit) return "00:00:00.000";
    
    double t = edit->getTransport().getPosition().inSeconds();
    int totalSeconds = static_cast<int>(t);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;
    int ms = static_cast<int>((t - totalSeconds) * 1000.0);
    
    return QString::asprintf("%02d:%02d:%02d.%03d", hours, minutes, seconds, ms);
}

double EngineController::bpm() const
{
    if (!edit) return 120.0;
    // Get BPM at current position
    return edit->tempoSequence.getTempoAt(edit->getTransport().getPosition()).getBpm();
}

void EngineController::togglePlay()
{
    if (!edit) return;
    
    auto& transport = edit->getTransport();
    if (transport.isPlaying())
        transport.stop(false, false);
    else
        transport.play(false);
}

void EngineController::stop()
{
    if (!edit) return;
    auto& transport = edit->getTransport();
    transport.stop(false, false);
    transport.setPosition(tracktion::TimePosition::fromSeconds(0.0));
}

void EngineController::setPositionSeconds(double seconds)
{
    if (!edit) return;
    edit->getTransport().setPosition(tracktion::TimePosition::fromSeconds(seconds));
}

void EngineController::toggleRecord()
{
    if (!edit) return;
    auto& transport = edit->getTransport();
    if (transport.isRecording())
        transport.record(false);
    else
        transport.record(true);
}

double EngineController::positionSeconds() const
{
    if (!edit) return 0.0;
    return edit->getTransport().getPosition().inSeconds();
}

void EngineController::showAudioSettings()
{
    // Use JUCE's AudioDeviceSelectorComponent in a DialogWindow
    auto* selector = new juce::AudioDeviceSelectorComponent(
        engine.getDeviceManager().deviceManager,
        0, 256, 0, 256,
        true, true, true, false);

    selector->setSize(500, 400);

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(selector);
    options.dialogTitle = "Audio Settings";
    options.dialogBackgroundColour = juce::Colour(0xff323336);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.launchAsync();
}

void EngineController::showPluginSettings()
{
    auto* pluginListComp = new juce::PluginListComponent(
        engine.getPluginManager().pluginFormatManager,
        engine.getPluginManager().knownPluginList,
        juce::File(),
        nullptr,
        true);

    pluginListComp->setSize(800, 600);

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(pluginListComp);
    options.dialogTitle = "Plugin Manager";
    options.dialogBackgroundColour = juce::Colour(0xff323336);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = true;
    options.launchAsync();
}

void EngineController::showNewProjectDialog()
{
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "New Project", "New Project dialog placeholder.");
}

void EngineController::showExportDialog()
{
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Export", "Export dialog placeholder.");
}

void EngineController::insertAudioFile()
{
    if (!edit) return;

    static std::unique_ptr<juce::FileChooser> chooser;
    chooser = std::make_unique<juce::FileChooser>("Select an Audio File", 
                                                  juce::File::getSpecialLocation(juce::File::userMusicDirectory), 
                                                  "*.wav;*.flac;*.mp3;*.ogg");
    
    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, 
                         [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file.existsAsFile())
        {
            auto tracks = tracktion::engine::getAudioTracks(*edit);
            if (!tracks.isEmpty())
            {
                auto track = tracks[0];
                tracktion::engine::AudioFile audioFile(engine, file);
                auto length = audioFile.getLength();
                auto pos = edit->getTransport().getPosition();
                auto len = tracktion::TimeDuration::fromSeconds(length);
                
                tracktion::engine::ClipPosition clipPos { { pos, pos + len }, tracktion::TimeDuration::fromSeconds(0) };
                
                track->insertWaveClip(file.getFileNameWithoutExtension(), 
                                      file, 
                                      clipPos, 
                                      false);
            }
        }
    });
}

void EngineController::insertAudioFileToTrack(int trackIndex, const QString& fileUrl, double positionSeconds)
{
    if (!edit) return;
    
    // QML gives us "file:///path/to/file.wav", so we need to convert to local file
    juce::File file(juce::URL(juce::String(fileUrl.toStdString())).getLocalFile());
    
    if (file.existsAsFile())
    {
        auto tracks = tracktion::engine::getAudioTracks(*edit);
        if (trackIndex >= 0 && trackIndex < tracks.size())
        {
            auto track = tracks[trackIndex];
            tracktion::engine::AudioFile audioFile(engine, file);
            auto length = audioFile.getLength();
            auto pos = tracktion::TimePosition::fromSeconds(positionSeconds);
            auto len = tracktion::TimeDuration::fromSeconds(length);
            
            tracktion::engine::ClipPosition clipPos { { pos, pos + len }, tracktion::TimeDuration::fromSeconds(0) };
            
            track->insertWaveClip(file.getFileNameWithoutExtension(), 
                                  file, 
                                  clipPos, 
                                  false);
        }
    }
}

QStringList EngineController::availablePlugins() const
{
    QStringList list;
    for (auto& type : engine.getPluginManager().knownPluginList.getTypes()) {
        list << QString::fromStdString(type.name.toStdString());
    }
    return list;
}

QStringList EngineController::audioDeviceTypes() const
{
    QStringList list;
    for (auto* type : engine.getDeviceManager().deviceManager.getAvailableDeviceTypes()) {
        list << QString::fromStdString(type->getTypeName().toStdString());
    }
    return list;
}

QString EngineController::currentDeviceType() const
{
    juce::String type = engine.getDeviceManager().deviceManager.getCurrentAudioDeviceType();
    return QString::fromStdString(type.toStdString());
}

QStringList EngineController::outputDevices() const
{
    QStringList list;
    if (auto* device = engine.getDeviceManager().deviceManager.getCurrentAudioDevice()) {
        auto names = device->getOutputChannelNames();
        for (int i = 0; i < names.size(); ++i) {
            list << QString::fromStdString(names[i].toStdString());
        }
    } else {
        // If no device, list devices for the current type
        if (auto* type = engine.getDeviceManager().deviceManager.getCurrentDeviceTypeObject()) {
            type->scanForDevices();
            auto names = type->getDeviceNames(false); // outputs
            for (int i = 0; i < names.size(); ++i) {
                list << QString::fromStdString(names[i].toStdString());
            }
        }
    }
    return list;
}

QString EngineController::currentOutputDevice() const
{
    juce::AudioDeviceManager::AudioDeviceSetup setup;
    engine.getDeviceManager().deviceManager.getAudioDeviceSetup(setup);
    return QString::fromStdString(setup.outputDeviceName.toStdString());
}

void EngineController::addPluginToTrack(int trackIndex, const QString& pluginName)
{
    if (!edit) return;
    auto tracks = tracktion::engine::getAudioTracks(*edit);
    if (trackIndex < 0 || trackIndex >= tracks.size()) return;
    
    // Find the plugin by name
    for (auto& type : engine.getPluginManager().knownPluginList.getTypes()) {
        if (type.name.toStdString() == pluginName.toStdString()) {
            auto pluginState = tracktion::engine::ExternalPlugin::create(engine, type);
            tracks[trackIndex]->pluginList.insertPlugin(pluginState, 0);
            break;
        }
    }
    
    Q_EMIT selectedTrackPluginsChanged(); // Emit so UI updates when a plugin is added
}

QStringList EngineController::selectedTrackPlugins() const
{
    QStringList list;
    if (!edit) return list;
    auto tracks = tracktion::engine::getAudioTracks(*edit);
    if (m_selectedTrackIndex < 0 || m_selectedTrackIndex >= tracks.size()) return list;
    
    auto* track = tracks[m_selectedTrackIndex];
    for (auto* plugin : track->pluginList) {
        list << QString::fromStdString(plugin->getName().toStdString());
    }
    return list;
}

void EngineController::setAudioDeviceType(const QString& typeName)
{
    engine.getDeviceManager().deviceManager.setCurrentAudioDeviceType(typeName.toStdString(), true);
    Q_EMIT audioDevicesChanged();
}

void EngineController::setOutputDevice(const QString& deviceName)
{
    juce::AudioDeviceManager::AudioDeviceSetup setup;
    engine.getDeviceManager().deviceManager.getAudioDeviceSetup(setup);
    setup.outputDeviceName = deviceName.toStdString();
    engine.getDeviceManager().deviceManager.setAudioDeviceSetup(setup, true);
    Q_EMIT audioDevicesChanged();
}

bool EngineController::metronomeEnabled() const {
    return edit ? edit->clickTrackEnabled.get() : false;
}

void EngineController::setMetronomeEnabled(bool enabled) {
    if (edit && edit->clickTrackEnabled.get() != enabled) {
        edit->clickTrackEnabled = enabled;
        Q_EMIT metronomeEnabledChanged();
    }
}

bool EngineController::masterMute() const {
    if (edit && edit->getMasterTrack())
        return edit->getMasterTrack()->isMuted(true);
    return false;
}

void EngineController::setMasterMute(bool mute) {
    if (edit && edit->getMasterTrack()) {
        edit->getMasterTrack()->setMute(mute);
        Q_EMIT masterMuteChanged();
    }
}

float EngineController::masterLevel() const {
    if (edit && edit->getTransport().isPlaying()) {
        if (masterMute()) return 0.0f;
        // Mock output level if playing
        return (std::rand() % 100) / 100.0f;
    }
    return 0.0f;
}

int EngineController::timeSigNumerator() const {
    if (edit && edit->tempoSequence.getNumTimeSigs() > 0)
        return edit->tempoSequence.getTimeSig(0)->numerator.get();
    return 4;
}

void EngineController::setTimeSigNumerator(int num) {
    if (edit && edit->tempoSequence.getNumTimeSigs() > 0) {
        edit->tempoSequence.getTimeSig(0)->numerator = num;
        Q_EMIT timeSigNumeratorChanged();
    }
}

int EngineController::timeSigDenominator() const {
    if (edit && edit->tempoSequence.getNumTimeSigs() > 0)
        return edit->tempoSequence.getTimeSig(0)->denominator.get();
    return 4;
}

void EngineController::setTimeSigDenominator(int den) {
    if (edit && edit->tempoSequence.getNumTimeSigs() > 0) {
        edit->tempoSequence.getTimeSig(0)->denominator = den;
        Q_EMIT timeSigDenominatorChanged();
    }
}

void EngineController::updateState()
{
    if (!edit) return;
    
    bool playing = edit->getTransport().isPlaying();
    if (playing != lastIsPlaying)
    {
        lastIsPlaying = playing;
        Q_EMIT isPlayingChanged();
    }
    
    bool recording = edit->getTransport().isRecording();
    if (recording != lastIsRecording)
    {
        lastIsRecording = recording;
        Q_EMIT isRecordingChanged();
    }
    
    QString tc = timecode();
    if (tc != lastTimecode)
    {
        lastTimecode = tc;
        Q_EMIT timecodeChanged();
    }
    
    double currentBpm = bpm();
    if (std::abs(currentBpm - lastBpm) > 0.001)
    {
        lastBpm = currentBpm;
        Q_EMIT bpmChanged();
    }

    double newPos = edit->getTransport().getPosition().inSeconds();
    if (newPos != m_positionSeconds) {
        m_positionSeconds = newPos;
        Q_EMIT positionSecondsChanged();
    }
    
    if (edit->getTransport().isPlaying()) {
        Q_EMIT masterLevelChanged();
    }
}
