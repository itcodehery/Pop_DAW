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

public:
    explicit EngineController(tracktion::engine::Engine& engine, 
                              tracktion::engine::Edit* edit, 
                              QObject* parent = nullptr);
    ~EngineController() override;

    bool isPlaying() const;
    bool isRecording() const;
    QString timecode() const;
    double bpm() const;

public Q_SLOTS:
    void togglePlay();
    void stop();
    void toggleRecord();

Q_SIGNALS:
    void isPlayingChanged();
    void isRecordingChanged();
    void timecodeChanged();
    void bpmChanged();

private Q_SLOTS:
    void updateState();

private:
    tracktion::engine::Engine& engine;
    tracktion::engine::Edit* edit;
    
    bool lastIsPlaying = false;
    bool lastIsRecording = false;
    QString lastTimecode;
    double lastBpm = 120.0;
    
    QTimer updateTimer;
};
