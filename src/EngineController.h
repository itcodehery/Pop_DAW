#pragma once

#include <QObject>
#include <QTimer>
#include <QString>
#include <memory>
#include <tracktion_engine/tracktion_engine.h>

class EngineController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY isRecordingChanged)
    Q_PROPERTY(bool isLooping READ isLooping WRITE setIsLooping NOTIFY loopChanged)
    Q_PROPERTY(double loopStartSeconds READ loopStartSeconds WRITE setLoopStartSeconds NOTIFY loopChanged)
    Q_PROPERTY(double loopEndSeconds READ loopEndSeconds WRITE setLoopEndSeconds NOTIFY loopChanged)
    Q_PROPERTY(QString timecode READ timecode NOTIFY timecodeChanged)
    Q_PROPERTY(double bpm READ bpm WRITE setBpm NOTIFY bpmChanged)
    Q_PROPERTY(double positionSeconds READ positionSeconds NOTIFY positionSecondsChanged)
    Q_PROPERTY(QString projectName READ projectName NOTIFY editChanged)

    // Plugins
    Q_PROPERTY(QStringList availablePlugins READ availablePlugins NOTIFY pluginsChanged)

    // Audio Devices
    Q_PROPERTY(QStringList audioDeviceTypes READ audioDeviceTypes NOTIFY audioDevicesChanged)
    Q_PROPERTY(QString currentDeviceType READ currentDeviceType NOTIFY audioDevicesChanged)
    Q_PROPERTY(QStringList outputDevices READ outputDevices NOTIFY audioDevicesChanged)
    Q_PROPERTY(QString currentOutputDevice READ currentOutputDevice NOTIFY audioDevicesChanged)
    Q_PROPERTY(QStringList inputDevices READ inputDevices NOTIFY audioDevicesChanged)
    Q_PROPERTY(QString currentInputDevice READ currentInputDevice NOTIFY audioDevicesChanged)

    // Master & Transport
    Q_PROPERTY(bool metronomeEnabled READ metronomeEnabled WRITE setMetronomeEnabled NOTIFY metronomeEnabledChanged)
    Q_PROPERTY(bool masterMute READ masterMute WRITE setMasterMute NOTIFY masterMuteChanged)
    Q_PROPERTY(float masterVolumeDb READ masterVolumeDb WRITE setMasterVolumeDb NOTIFY masterVolumeDbChanged)
    Q_PROPERTY(float masterLevel READ masterLevel NOTIFY masterLevelChanged)
    Q_PROPERTY(int timeSigNumerator READ timeSigNumerator WRITE setTimeSigNumerator NOTIFY timeSigNumeratorChanged)
    Q_PROPERTY(int timeSigDenominator READ timeSigDenominator WRITE setTimeSigDenominator NOTIFY timeSigDenominatorChanged)
    
    // Selection state
    Q_PROPERTY(int selectedTrackIndex READ selectedTrackIndex WRITE setSelectedTrackIndex NOTIFY selectedTrackIndexChanged)
    Q_PROPERTY(int selectedClipIndex READ selectedClipIndex WRITE setSelectedClipIndex NOTIFY selectedClipIndexChanged)
    Q_PROPERTY(QStringList selectedTrackPlugins READ selectedTrackPlugins NOTIFY selectedTrackPluginsChanged)

public:
    explicit EngineController(tracktion::engine::Engine& engine, 
                              std::unique_ptr<tracktion::engine::Edit> edit, 
                              QObject* parent = nullptr);
    ~EngineController() override;

    bool isPlaying() const;
    bool isRecording() const;
    bool isLooping() const;
    void setIsLooping(bool loop);
    double loopStartSeconds() const;
    void setLoopStartSeconds(double s);
    double loopEndSeconds() const;
    void setLoopEndSeconds(double s);

    QString timecode() const;
    double bpm() const;
    void setBpm(double newBpm);
    double positionSeconds() const;
    QString projectName() const;
    
    tracktion::engine::Edit* getEdit() const { return edit.get(); }
    tracktion::engine::Engine& getEngine() const { return engine; }
    
    QStringList availablePlugins() const;
    QStringList audioDeviceTypes() const;
    QString currentDeviceType() const;
    QStringList outputDevices() const;
    QString currentOutputDevice() const;
    QStringList inputDevices() const;
    QString currentInputDevice() const;
    
    bool metronomeEnabled() const;
    void setMetronomeEnabled(bool enabled);
    
    bool masterMute() const;
    void setMasterMute(bool mute);
    
    float masterVolumeDb() const;
    void setMasterVolumeDb(float db);
    float masterLevel() const { return m_masterLevel; }
    
    int timeSigNumerator() const;
    void setTimeSigNumerator(int num);
    
    int timeSigDenominator() const;
    void setTimeSigDenominator(int den);
    
    int selectedTrackIndex() const { return m_selectedTrackIndex; }
    void setSelectedTrackIndex(int i) { 
        if (m_selectedTrackIndex != i) {
            m_selectedTrackIndex = i; 
            m_selectedClipIndex = -1;
            Q_EMIT selectedTrackIndexChanged(); 
            Q_EMIT selectedClipIndexChanged();
            Q_EMIT selectedTrackPluginsChanged();
        }
    }

    int selectedClipIndex() const { return m_selectedClipIndex; }
    void setSelectedClipIndex(int i) {
        if (m_selectedClipIndex != i) {
            m_selectedClipIndex = i;
            Q_EMIT selectedClipIndexChanged();
        }
    }
    
    QStringList selectedTrackPlugins() const;
    Q_INVOKABLE void showPluginEditor(int trackIndex, int pluginIndex);
    Q_INVOKABLE void removePluginFromTrack(int trackIndex, int pluginIndex);

public Q_SLOTS:
    void togglePlay();
    void stop();
    void rewindToStart();
    void skipForward();
    void toggleRecord();
    void undo();
    void redo();
    void saveProject();
    void saveProjectAs();
    void openProject();
    void showNewProjectDialog();
    void showExportDialog();
    void showAudioSettings();
    void showPluginSettings();
    void setPositionSeconds(double seconds);
    void insertAudioFile();
    void insertAudioFileToTrack(int trackIndex, const QString& fileUrl, double positionSeconds);
    
    void addPluginToTrack(int trackIndex, const QString& pluginName);
    void setAudioDeviceType(const QString& typeName);
    void setOutputDevice(const QString& deviceName);
    void setInputDevice(const QString& deviceName);

Q_SIGNALS:
    void editChanged();
    void isPlayingChanged();
    void isRecordingChanged();
    void loopChanged();
    void timecodeChanged();
    void bpmChanged();
    void positionSecondsChanged();
    void pluginsChanged();
    void audioDevicesChanged();
    
    void metronomeEnabledChanged();
    void masterMuteChanged();
    void masterVolumeDbChanged();
    void masterLevelChanged();
    void timeSigNumeratorChanged();
    void timeSigDenominatorChanged();
    void selectedTrackIndexChanged();
    void selectedClipIndexChanged();
    void selectedTrackPluginsChanged();
    void statusMessage(const QString& message);

private Q_SLOTS:
    void updateState();

private:
    void loadOrReplaceEdit(std::unique_ptr<tracktion::engine::Edit> newEdit);
    void ensureWaveInputsEnabled();

    tracktion::engine::Engine& engine;
    std::unique_ptr<tracktion::engine::Edit> edit;
    
    tracktion::engine::LevelMeasurer::Client masterLevelClient;
    juce::WeakReference<tracktion::engine::LevelMeasurer> attachedMasterMeasurer;
    float m_masterLevel = 0.0f;

    QTimer updateTimer;
    bool lastIsPlaying = false;
    bool lastIsRecording = false;
    QString lastTimecode;
    double lastBpm = 120.0;
    double m_positionSeconds = 0.0;
    
    int m_selectedTrackIndex = 0;
    int m_selectedClipIndex = -1;
};
