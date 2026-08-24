#include "EngineController.h"

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

void EngineController::toggleRecord()
{
    if (!edit) return;
    auto& transport = edit->getTransport();
    if (transport.isRecording())
        transport.record(false);
    else
        transport.record(true);
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
}
