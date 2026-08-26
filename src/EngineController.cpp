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

    double currentPos = positionSeconds();
    if (std::abs(currentPos - lastPositionSeconds) > 0.01)
    {
        lastPositionSeconds = currentPos;
        Q_EMIT positionSecondsChanged();
    }
}
