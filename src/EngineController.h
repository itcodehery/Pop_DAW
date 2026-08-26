#pragma once

#include <QObject>
#include <QTimer>
#include <QString>
#include <tracktion_engine/tracktion_engine.h>

class EngineController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY isRecordingChanged)
    Q_PROPERTY(QString timecode READ timecode NOTIFY timecodeChanged)
    Q_PROPERTY(double bpm READ bpm NOTIFY bpmChanged)
    Q_PROPERTY(double positionSeconds READ positionSeconds NOTIFY positionSecondsChanged)

    // Plugins
    Q_PROPERTY(QStringList availablePlugins READ availablePlugins NOTIFY pluginsChanged)

    // Audio Devices
    Q_PROPERTY(QStringList audioDeviceTypes READ audioDeviceTypes NOTIFY audioDevicesChanged)
    Q_PROPERTY(QString currentDeviceType READ currentDeviceType NOTIFY audioDevicesChanged)
    Q_PROPERTY(QStringList outputDevices READ outputDevices NOTIFY audioDevicesChanged)
    Q_PROPERTY(QString currentOutputDevice READ currentOutputDevice NOTIFY audioDevicesChanged)

    // Master & Transport
    Q_PROPERTY(bool metronomeEnabled READ metronomeEnabled WRITE setMetronomeEnabled NOTIFY metronomeEnabledChanged)
    Q_PROPERTY(bool masterMute READ masterMute WRITE setMasterMute NOTIFY masterMuteChanged)
    Q_PROPERTY(float masterLevel READ masterLevel NOTIFY masterLevelChanged)
    Q_PROPERTY(int timeSigNumerator READ timeSigNumerator WRITE setTimeSigNumerator NOTIFY timeSigNumeratorChanged)
    Q_PROPERTY(int timeSigDenominator READ timeSigDenominator WRITE setTimeSigDenominator NOTIFY timeSigDenominatorChanged)
    
    Q_PROPERTY(int selectedTrackIndex READ selectedTrackIndex WRITE setSelectedTrackIndex NOTIFY selectedTrackIndexChanged)
    Q_PROPERTY(QStringList selectedTrackPlugins READ selectedTrackPlugins NOTIFY selectedTrackPluginsChanged)

public:
    explicit EngineController(tracktion::engine::Engine& engine, 
                              tracktion::engine::Edit* edit, 
                              QObject* parent = nullptr);
    ~EngineController() override;

    bool isPlaying() const;
    bool isRecording() const;
    QString timecode() const;
    double bpm() const;
    double positionSeconds() const;
    
    tracktion::engine::Edit* getEdit() const { return edit; }
    
    QStringList availablePlugins() const;
    QStringList audioDeviceTypes() const;
    QString currentDeviceType() const;
    QStringList outputDevices() const;
    QString currentOutputDevice() const;
    
    bool metronomeEnabled() const;
    void setMetronomeEnabled(bool enabled);
    
    bool masterMute() const;
    void setMasterMute(bool mute);
    
    float masterLevel() const;
    
    int timeSigNumerator() const;
    void setTimeSigNumerator(int num);
    
    int timeSigDenominator() const;
    void setTimeSigDenominator(int den);
    
    int selectedTrackIndex() const { return m_selectedTrackIndex; }
    void setSelectedTrackIndex(int i) { 
        if (m_selectedTrackIndex != i) {
            m_selectedTrackIndex = i; 
            Q_EMIT selectedTrackIndexChanged(); 
            Q_EMIT selectedTrackPluginsChanged();
        }
    }
    
    QStringList selectedTrackPlugins() const;

public Q_SLOTS:
    void togglePlay();
    void stop();
    void toggleRecord();
    void showAudioSettings();
    void showPluginSettings();
    void showNewProjectDialog();
    void showExportDialog();
    void setPositionSeconds(double seconds);
    void insertAudioFile();
    void insertAudioFileToTrack(int trackIndex, const QString& fileUrl, double positionSeconds);
    
    void addPluginToTrack(int trackIndex, const QString& pluginName);
    void setAudioDeviceType(const QString& typeName);
    void setOutputDevice(const QString& deviceName);

Q_SIGNALS:
    void isPlayingChanged();
    void isRecordingChanged();
    void timecodeChanged();
    void bpmChanged();
    void positionSecondsChanged();
    void pluginsChanged();
    void audioDevicesChanged();
    
    void metronomeEnabledChanged();
    void masterMuteChanged();
    void masterLevelChanged();
    void timeSigNumeratorChanged();
    void timeSigDenominatorChanged();
    void selectedTrackIndexChanged();
    void selectedTrackPluginsChanged();

private Q_SLOTS:
    void updateState();

private:
    tracktion::engine::Engine& engine;
    tracktion::engine::Edit* edit;
    
    QTimer updateTimer;
    bool lastIsPlaying = false;
    bool lastIsRecording = false;
    QString lastTimecode;
    double lastBpm = 120.0;
    double lastPositionSeconds = 0.0;
    double m_positionSeconds = 0.0;
    
    int m_selectedTrackIndex = 0;
};
