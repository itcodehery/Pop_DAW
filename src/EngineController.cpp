#include "EngineController.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <tracktion_engine/tracktion_engine.h>
#include <cmath>
#include <algorithm>

EngineController::EngineController(tracktion::engine::Engine& eng, std::unique_ptr<tracktion::engine::Edit> ed, QObject* parent)
    : QObject(parent), engine(eng), edit(std::move(ed))
{
    ensureWaveInputsEnabled();
    if (edit) {
        if (tracktion::engine::getAudioTracks(*edit).isEmpty()) {
            edit->ensureNumberOfAudioTracks(2);
        }
    }

    connect(&updateTimer, &QTimer::timeout, this, &EngineController::updateState);
    updateTimer.start(30); // ~30 FPS UI updates
}

EngineController::~EngineController()
{
    if (attachedMasterMeasurer != nullptr)
        attachedMasterMeasurer->removeClient(masterLevelClient);
    if (edit)
        tracktion::engine::EditFileOperations(*edit).save(true, true, false);
}

void EngineController::ensureWaveInputsEnabled()
{
    auto& dm = engine.getDeviceManager();
    for (int i = 0; i < dm.getNumWaveInDevices(); ++i) {
        if (auto* wip = dm.getWaveInDevice(i)) {
            wip->setMonitorMode(tracktion::engine::InputDevice::MonitorMode::automatic);
            wip->setEnabled(true);
        }
    }
    if (edit) {
        edit->getTransport().ensureContextAllocated();
    }
}

void EngineController::loadOrReplaceEdit(std::unique_ptr<tracktion::engine::Edit> newEdit)
{
    if (!newEdit) return;
    if (attachedMasterMeasurer != nullptr) {
        attachedMasterMeasurer->removeClient(masterLevelClient);
        attachedMasterMeasurer = nullptr;
    }
    if (edit) {
        edit->getTransport().stop(false, false);
    }

    edit = std::move(newEdit);
    edit->playInStopEnabled = true;
    ensureWaveInputsEnabled();

    if (tracktion::engine::getAudioTracks(*edit).isEmpty())
        edit->ensureNumberOfAudioTracks(2);

    m_selectedTrackIndex = 0;
    m_selectedClipIndex = -1;

    Q_EMIT editChanged();
    Q_EMIT selectedTrackIndexChanged();
    Q_EMIT selectedClipIndexChanged();
    Q_EMIT selectedTrackPluginsChanged();
    Q_EMIT bpmChanged();
    Q_EMIT timeSigNumeratorChanged();
    Q_EMIT timeSigDenominatorChanged();
    Q_EMIT metronomeEnabledChanged();
    Q_EMIT masterMuteChanged();
    Q_EMIT masterVolumeDbChanged();
    Q_EMIT loopChanged();
}

QString EngineController::projectName() const
{
    if (!edit) return "Untitled";
    auto f = tracktion::engine::EditFileOperations(*edit).getEditFile();
    if (f != juce::File())
        return QString::fromStdString(f.getFileNameWithoutExtension().toStdString());
    return "Untitled";
}

bool EngineController::isPlaying() const
{
    return edit ? edit->getTransport().isPlaying() : false;
}

bool EngineController::isRecording() const
{
    return edit ? edit->getTransport().isRecording() : false;
}

bool EngineController::isLooping() const
{
    return edit ? edit->getTransport().looping.get() : false;
}

void EngineController::setIsLooping(bool loop)
{
    if (!edit) return;
    auto& transport = edit->getTransport();
    if (transport.looping.get() != loop) {
        if (loop && transport.getLoopRange().getLength().inSeconds() < 0.5) {
            transport.setLoopRange({ tracktion::TimePosition::fromSeconds(0.0),
                                     tracktion::TimePosition::fromSeconds(8.0) });
        }
        transport.looping = loop;
        Q_EMIT loopChanged();
    }
}

double EngineController::loopStartSeconds() const
{
    return edit ? edit->getTransport().getLoopRange().getStart().inSeconds() : 0.0;
}

void EngineController::setLoopStartSeconds(double s)
{
    if (!edit) return;
    auto range = edit->getTransport().getLoopRange();
    double end = std::max(s + 0.25, range.getEnd().inSeconds());
    edit->getTransport().setLoopRange({ tracktion::TimePosition::fromSeconds(std::max(0.0, s)),
                                        tracktion::TimePosition::fromSeconds(end) });
    Q_EMIT loopChanged();
}

double EngineController::loopEndSeconds() const
{
    return edit ? edit->getTransport().getLoopRange().getEnd().inSeconds() : 8.0;
}

void EngineController::setLoopEndSeconds(double s)
{
    if (!edit) return;
    auto range = edit->getTransport().getLoopRange();
    double start = range.getStart().inSeconds();
    double end = std::max(start + 0.25, s);
    edit->getTransport().setLoopRange({ tracktion::TimePosition::fromSeconds(start),
                                        tracktion::TimePosition::fromSeconds(end) });
    Q_EMIT loopChanged();
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
    return edit->tempoSequence.getTempoAt(edit->getTransport().getPosition()).getBpm();
}

void EngineController::setBpm(double newBpm)
{
    if (!edit) return;
    double clamped = std::clamp(newBpm, 20.0, 400.0);
    if (edit->tempoSequence.getNumTempos() > 0) {
        edit->tempoSequence.getTempo(0)->setBpm(clamped);
        Q_EMIT bpmChanged();
    }
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

void EngineController::rewindToStart()
{
    if (!edit) return;
    edit->getTransport().setPosition(tracktion::TimePosition::fromSeconds(0.0));
}

void EngineController::skipForward()
{
    if (!edit) return;
    auto& transport = edit->getTransport();
    double pos = transport.getPosition().inSeconds();
    transport.setPosition(tracktion::TimePosition::fromSeconds(pos + 4.0));
}

void EngineController::setPositionSeconds(double seconds)
{
    if (!edit) return;
    edit->getTransport().setPosition(tracktion::TimePosition::fromSeconds(std::max(0.0, seconds)));
}

void EngineController::toggleRecord()
{
    if (!edit) return;
    auto& transport = edit->getTransport();
    if (transport.isRecording()) {
        transport.stop(true, false);
        tracktion::engine::EditFileOperations(*edit).save(true, true, false);
    } else {
        // Ensure at least the selected track is armed if none are armed
        bool anyArmed = false;
        auto tracks = tracktion::engine::getAudioTracks(*edit);
        for (auto* t : tracks) {
            for (auto* inst : edit->getAllInputDevices()) {
                if (tracktion::engine::isOnTargetTrack(*inst, *t, 0) && inst->isRecordingEnabled(t->itemID)) {
                    anyArmed = true;
                    break;
                }
            }
        }
        if (!anyArmed && !tracks.isEmpty()) {
            int idx = (m_selectedTrackIndex >= 0 && m_selectedTrackIndex < tracks.size()) ? m_selectedTrackIndex : 0;
            auto* t = tracks[idx];
            for (auto* inst : edit->getAllInputDevices()) {
                if (inst->getInputDevice().getDeviceType() == tracktion::engine::InputDevice::waveDevice) {
                    [[maybe_unused]] auto res = inst->setTarget(t->itemID, true, &edit->getUndoManager(), 0);
                    inst->setRecordingEnabled(t->itemID, true);
                    break;
                }
            }
        }
        transport.record(false);
    }
}

void EngineController::undo()
{
    if (edit) {
        edit->getUndoManager().undo();
        Q_EMIT statusMessage("Undo");
    }
}

void EngineController::redo()
{
    if (edit) {
        edit->getUndoManager().redo();
        Q_EMIT statusMessage("Redo");
    }
}

double EngineController::positionSeconds() const
{
    if (!edit) return 0.0;
    return edit->getTransport().getPosition().inSeconds();
}

void EngineController::showAudioSettings()
{
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

void EngineController::saveProject()
{
    if (!edit) return;
    tracktion::engine::EditFileOperations(*edit).save(true, true, false);
    Q_EMIT statusMessage("Saved project: " + projectName());
}

void EngineController::saveProjectAs()
{
    if (!edit) return;
    static std::unique_ptr<juce::FileChooser> chooser;
    chooser = std::make_unique<juce::FileChooser>("Save Project As",
                                                  juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                                                  "*.tracktionedit");
    chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                         [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file != juce::File()) {
            auto dest = file.withFileExtension(".tracktionedit");
            tracktion::engine::EditFileOperations(*edit).saveAs(dest, true, nullptr);
            Q_EMIT editChanged();
            Q_EMIT statusMessage("Saved as " + QString::fromStdString(dest.getFileName().toStdString()));
        }
    });
}

void EngineController::openProject()
{
    static std::unique_ptr<juce::FileChooser> chooser;
    chooser = std::make_unique<juce::FileChooser>("Open Pop DAW Session",
                                                  juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                                                  "*.tracktionedit");
    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                         [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file.existsAsFile()) {
            auto loaded = tracktion::engine::loadEditFromFile(engine, file);
            if (loaded) {
                loadOrReplaceEdit(std::move(loaded));
                Q_EMIT statusMessage("Opened " + QString::fromStdString(file.getFileName().toStdString()));
            }
        }
    });
}

void EngineController::showNewProjectDialog()
{
    static std::unique_ptr<juce::FileChooser> chooser;
    chooser = std::make_unique<juce::FileChooser>("Create New Session",
                                                  juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Untitled.tracktionedit"),
                                                  "*.tracktionedit");
    chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                         [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file != juce::File()) {
            auto dest = file.withFileExtension(".tracktionedit");
            if (dest.existsAsFile())
                dest.deleteFile();
            auto newEd = tracktion::engine::createEmptyEdit(engine, dest);
            loadOrReplaceEdit(std::move(newEd));
            Q_EMIT statusMessage("Created new project: " + QString::fromStdString(dest.getFileNameWithoutExtension().toStdString()));
        }
    });
}

void EngineController::showExportDialog()
{
    if (!edit) return;

    static std::unique_ptr<juce::FileChooser> chooser;
    chooser = std::make_unique<juce::FileChooser>("Export Audio Mixdown",
                                                  juce::File::getSpecialLocation(juce::File::userMusicDirectory).getChildFile("Mixdown.wav"),
                                                  "*.wav;*.flac;*.ogg");

    chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                         [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file == juce::File() || !edit) return;

        if (!file.hasFileExtension("wav;flac;ogg"))
            file = file.withFileExtension(".wav");

        if (file.existsAsFile())
            file.deleteFile();

        edit->getTransport().stop(false, false);

        // Determine edit length or loop range
        tracktion::TimeRange renderRange { tracktion::TimePosition::fromSeconds(0.0), edit->getLength() };
        if (edit->getTransport().looping.get() && edit->getTransport().getLoopRange().getLength().inSeconds() > 0.1) {
            renderRange = edit->getTransport().getLoopRange();
        }
        if (renderRange.getLength().inSeconds() <= 0.05) {
            Q_EMIT statusMessage("Nothing to export (timeline is empty)");
            return;
        }

        tracktion::engine::Renderer::Parameters params(*edit);
        params.destFile = file;
        params.time = renderRange;
        params.bitDepth = 24;
        params.sampleRateForAudio = 44100.0;
        params.usePlugins = true;
        params.useMasterPlugins = true;

        if (file.hasFileExtension("flac")) {
            params.audioFormat = engine.getAudioFileFormatManager().getFlacFormat();
        } else if (file.hasFileExtension("ogg")) {
            params.audioFormat = engine.getAudioFileFormatManager().getOggFormat();
        } else {
            params.audioFormat = engine.getAudioFileFormatManager().getWavFormat();
        }

        juce::BigInteger tracksToDo;
        int idx = 0;
        for (auto* t : tracktion::engine::getAllTracks(*edit)) {
            tracksToDo.setBit(idx++);
            (void)t;
        }
        params.tracksToDo = tracksToDo;

        auto renderedFile = tracktion::engine::Renderer::renderToFile("Exporting Audio", params);
        if (renderedFile.existsAsFile()) {
            Q_EMIT statusMessage("Exported to " + QString::fromStdString(renderedFile.getFullPathName().toStdString()));
        } else {
            Q_EMIT statusMessage("Export failed");
        }
    });
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
        if (file.existsAsFile() && edit)
        {
            auto tracks = tracktion::engine::getAudioTracks(*edit);
            if (tracks.isEmpty()) {
                edit->ensureNumberOfAudioTracks(1);
                tracks = tracktion::engine::getAudioTracks(*edit);
            }
            if (!tracks.isEmpty())
            {
                int targetIndex = (m_selectedTrackIndex >= 0 && m_selectedTrackIndex < tracks.size()) ? m_selectedTrackIndex : 0;
                auto track = tracks[targetIndex];
                tracktion::engine::AudioFile audioFile(engine, file);
                auto length = audioFile.getLength();
                if (length <= 0.0) return;

                auto pos = edit->getTransport().getPosition();
                auto len = tracktion::TimeDuration::fromSeconds(length);
                
                tracktion::engine::ClipPosition clipPos { { pos, pos + len }, tracktion::TimeDuration::fromSeconds(0) };
                
                track->insertWaveClip(file.getFileNameWithoutExtension(), 
                                      file, 
                                      clipPos, 
                                      false);
                Q_EMIT statusMessage("Inserted " + QString::fromStdString(file.getFileName().toStdString()));
            }
        }
    });
}

void EngineController::insertAudioFileToTrack(int trackIndex, const QString& fileUrl, double positionSeconds)
{
    if (!edit) return;
    
    juce::String pathStr = fileUrl.toStdString();
    juce::File file;
    if (pathStr.startsWith("file://")) {
        file = juce::URL(pathStr).getLocalFile();
    } else {
        file = juce::File(pathStr);
    }
    
    if (file.existsAsFile())
    {
        auto tracks = tracktion::engine::getAudioTracks(*edit);
        if (trackIndex >= 0 && trackIndex < tracks.size())
        {
            auto track = tracks[trackIndex];
            tracktion::engine::AudioFile audioFile(engine, file);
            auto length = audioFile.getLength();
            if (length <= 0.0) return;

            auto pos = tracktion::TimePosition::fromSeconds(std::max(0.0, positionSeconds));
            auto len = tracktion::TimeDuration::fromSeconds(length);
            
            tracktion::engine::ClipPosition clipPos { { pos, pos + len }, tracktion::TimeDuration::fromSeconds(0) };
            
            track->insertWaveClip(file.getFileNameWithoutExtension(), 
                                  file, 
                                  clipPos, 
                                  false);
            Q_EMIT statusMessage("Dropped " + QString::fromStdString(file.getFileName().toStdString()));
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
    if (auto* type = engine.getDeviceManager().deviceManager.getCurrentDeviceTypeObject()) {
        type->scanForDevices();
        auto names = type->getDeviceNames(false);
        for (int i = 0; i < names.size(); ++i)
            list << QString::fromStdString(names[i].toStdString());
    }
    return list;
}

QString EngineController::currentOutputDevice() const
{
    juce::AudioDeviceManager::AudioDeviceSetup setup;
    engine.getDeviceManager().deviceManager.getAudioDeviceSetup(setup);
    return QString::fromStdString(setup.outputDeviceName.toStdString());
}

QStringList EngineController::inputDevices() const
{
    QStringList list;
    if (auto* type = engine.getDeviceManager().deviceManager.getCurrentDeviceTypeObject()) {
        type->scanForDevices();
        auto names = type->getDeviceNames(true);
        for (int i = 0; i < names.size(); ++i)
            list << QString::fromStdString(names[i].toStdString());
    }
    return list;
}

QString EngineController::currentInputDevice() const
{
    juce::AudioDeviceManager::AudioDeviceSetup setup;
    engine.getDeviceManager().deviceManager.getAudioDeviceSetup(setup);
    return QString::fromStdString(setup.inputDeviceName.toStdString());
}

void EngineController::addPluginToTrack(int trackIndex, const QString& pluginName)
{
    if (!edit) return;
    auto tracks = tracktion::engine::getAudioTracks(*edit);
    if (trackIndex < 0 || trackIndex >= tracks.size()) return;
    
    for (auto& type : engine.getPluginManager().knownPluginList.getTypes()) {
        if (type.name.toStdString() == pluginName.toStdString()) {
            auto pluginState = tracktion::engine::ExternalPlugin::create(engine, type);
            tracks[trackIndex]->pluginList.insertPlugin(pluginState, 0);
            break;
        }
    }
    
    Q_EMIT selectedTrackPluginsChanged();
}

void EngineController::removePluginFromTrack(int trackIndex, int pluginIndex)
{
    if (!edit) return;
    auto tracks = tracktion::engine::getAudioTracks(*edit);
    if (trackIndex < 0 || trackIndex >= tracks.size()) return;

    auto* track = tracks[trackIndex];
    int userIdx = 0;
    for (auto* plugin : track->pluginList) {
        if (dynamic_cast<tracktion::engine::VolumeAndPanPlugin*>(plugin) != nullptr ||
            dynamic_cast<tracktion::engine::LevelMeterPlugin*>(plugin) != nullptr)
            continue;
        if (userIdx == pluginIndex) {
            plugin->deleteFromParent();
            break;
        }
        ++userIdx;
    }
    Q_EMIT selectedTrackPluginsChanged();
}

QStringList EngineController::selectedTrackPlugins() const
{
    QStringList list;
    if (!edit) return list;
    auto tracks = tracktion::engine::getAudioTracks(*edit);
    if (m_selectedTrackIndex < 0 || m_selectedTrackIndex >= tracks.size()) return list;
    
    auto* track = tracks[m_selectedTrackIndex];
    for (auto* plugin : track->pluginList) {
        if (dynamic_cast<tracktion::engine::VolumeAndPanPlugin*>(plugin) != nullptr ||
            dynamic_cast<tracktion::engine::LevelMeterPlugin*>(plugin) != nullptr)
            continue;
        list << QString::fromStdString(plugin->getName().toStdString());
    }
    return list;
}

class PluginWindow : public juce::DocumentWindow {
public:
    PluginWindow(const juce::String& name, tracktion::engine::Plugin::Ptr p)
        : DocumentWindow(name, juce::Colours::darkgrey, DocumentWindow::allButtons), plugin(p)
    {
        setUsingNativeTitleBar(true);
        if (auto editor = plugin->createEditor()) {
            const bool canResize = editor->allowWindowResizing();
            setContentOwned(editor.release(), true);
            setResizable(canResize, false);
        } else {
            auto* label = new juce::Label(juce::String(), "This plugin has no GUI.");
            label->setSize(300, 200);
            label->setJustificationType(juce::Justification::centred);
            setContentOwned(label, true);
        }
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
    }
    
    void closeButtonPressed() override {
        delete this;
    }
private:
    tracktion::engine::Plugin::Ptr plugin;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginWindow)
};

void EngineController::showPluginEditor(int trackIndex, int pluginIndex)
{
    if (!edit) return;
    auto tracks = tracktion::engine::getAudioTracks(*edit);
    if (trackIndex < 0 || trackIndex >= tracks.size()) return;
    
    auto* track = tracks[trackIndex];
    int userIdx = 0;
    for (auto* plugin : track->pluginList) {
        if (dynamic_cast<tracktion::engine::VolumeAndPanPlugin*>(plugin) != nullptr ||
            dynamic_cast<tracktion::engine::LevelMeterPlugin*>(plugin) != nullptr)
            continue;
        if (userIdx == pluginIndex) {
            new PluginWindow(plugin->getName(), plugin);
            return;
        }
        ++userIdx;
    }
}

void EngineController::setAudioDeviceType(const QString& typeName)
{
    engine.getDeviceManager().deviceManager.setCurrentAudioDeviceType(typeName.toStdString(), true);
    ensureWaveInputsEnabled();
    Q_EMIT audioDevicesChanged();
}

void EngineController::setOutputDevice(const QString& deviceName)
{
    juce::AudioDeviceManager::AudioDeviceSetup setup;
    engine.getDeviceManager().deviceManager.getAudioDeviceSetup(setup);
    setup.outputDeviceName = deviceName.toStdString();
    engine.getDeviceManager().deviceManager.setAudioDeviceSetup(setup, true);
    ensureWaveInputsEnabled();
    Q_EMIT audioDevicesChanged();
}

void EngineController::setInputDevice(const QString& deviceName)
{
    juce::AudioDeviceManager::AudioDeviceSetup setup;
    engine.getDeviceManager().deviceManager.getAudioDeviceSetup(setup);
    setup.inputDeviceName = deviceName.toStdString();
    setup.useDefaultInputChannels = true;
    engine.getDeviceManager().deviceManager.setAudioDeviceSetup(setup, true);
    ensureWaveInputsEnabled();
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
    if (edit) {
        if (auto mvp = edit->getMasterVolumePlugin())
            return mvp->getVolumeDb() <= -90.0f;
    }
    return false;
}

void EngineController::setMasterMute(bool mute) {
    if (edit) {
        if (auto mvp = edit->getMasterVolumePlugin()) {
            mvp->setVolumeDb(mute ? -100.0f : 0.0f);
            Q_EMIT masterMuteChanged();
            Q_EMIT masterVolumeDbChanged();
        }
    }
}

float EngineController::masterVolumeDb() const {
    if (edit) {
        if (auto mvp = edit->getMasterVolumePlugin())
            return mvp->getVolumeDb();
    }
    return 0.0f;
}

void EngineController::setMasterVolumeDb(float db) {
    if (edit) {
        if (auto mvp = edit->getMasterVolumePlugin()) {
            mvp->setVolumeDb(std::clamp(db, -100.0f, 6.0f));
            Q_EMIT masterVolumeDbChanged();
            Q_EMIT masterMuteChanged();
        }
    }
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
    if (std::abs(newPos - m_positionSeconds) > 0.0001) {
        m_positionSeconds = newPos;
        Q_EMIT positionSecondsChanged();
    }
    
    // Update real master audio levels from playback context
    if (auto* ctx = edit->getCurrentPlaybackContext()) {
        if (&ctx->masterLevels != attachedMasterMeasurer.get()) {
            if (attachedMasterMeasurer != nullptr)
                attachedMasterMeasurer->removeClient(masterLevelClient);
            attachedMasterMeasurer = &ctx->masterLevels;
            attachedMasterMeasurer->addClient(masterLevelClient);
        }
        float dbL = masterLevelClient.getAndClearAudioLevel(0).dB;
        float dbR = masterLevelClient.getAndClearAudioLevel(1).dB;
        float maxDb = std::max(dbL, dbR);
        float targetLevel = std::clamp((maxDb + 60.0f) / 66.0f, 0.0f, 1.0f);
        if (masterMute()) targetLevel = 0.0f;
        m_masterLevel = m_masterLevel * 0.6f + targetLevel * 0.4f;
        Q_EMIT masterLevelChanged();
    } else if (m_masterLevel > 0.001f) {
        m_masterLevel *= 0.7f;
        Q_EMIT masterLevelChanged();
    }
}
