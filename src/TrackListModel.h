#pragma once

#include <QAbstractListModel>
#include <QColor>
#include <QTimer>
#include <memory>
#include <vector>
#include <tracktion_engine/tracktion_engine.h>

class EngineController;
class ClipListModel;

class TrackListModel : public QAbstractListModel, private tracktion::engine::ValueTreeAllEventListener
{
    Q_OBJECT

public:
    enum TrackRoles {
        NameRole = Qt::UserRole + 1,
        ColorRole,
        MuteRole,
        SoloRole,
        ArmedRole,
        VolumeRole,
        PanRole,
        LevelRole,
        ClipsRole
    };

    explicit TrackListModel(EngineController* controller, QObject* parent = nullptr);
    ~TrackListModel() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    // Track operations
    Q_INVOKABLE void addTrack();
    Q_INVOKABLE void removeTrack(int index);
    Q_INVOKABLE void renameTrack(int index, const QString& newName);
    Q_INVOKABLE void toggleTrackMute(int index);
    Q_INVOKABLE void toggleTrackSolo(int index);
    Q_INVOKABLE void toggleTrackArm(int index);
    Q_INVOKABLE void setTrackVolume(int index, float db);
    Q_INVOKABLE void setTrackPan(int index, float pan);
    Q_INVOKABLE void addMidiClip(int trackIndex, double startTime, double length);

    // Clip editing operations
    Q_INVOKABLE void moveClip(int trackIndex, int clipIndex, double newStartSeconds);
    Q_INVOKABLE void trimClipLeft(int trackIndex, int clipIndex, double newStartSeconds);
    Q_INVOKABLE void trimClipRight(int trackIndex, int clipIndex, double newLengthSeconds);
    Q_INVOKABLE void setClipFadeIn(int trackIndex, int clipIndex, double fadeInSeconds);
    Q_INVOKABLE void setClipFadeOut(int trackIndex, int clipIndex, double fadeOutSeconds);
    Q_INVOKABLE void setClipGainDb(int trackIndex, int clipIndex, float gainDb);
    Q_INVOKABLE void toggleClipReverse(int trackIndex, int clipIndex);
    Q_INVOKABLE void setClipPitch(int trackIndex, int clipIndex, float semitones);
    Q_INVOKABLE void setClipSpeed(int trackIndex, int clipIndex, double ratio);
    Q_INVOKABLE void normalizeClip(int trackIndex, int clipIndex);
    Q_INVOKABLE void splitClip(int trackIndex, int clipIndex, double splitTimeSeconds);
    Q_INVOKABLE void duplicateClip(int trackIndex, int clipIndex);
    Q_INVOKABLE void copyClip(int trackIndex, int clipIndex);
    Q_INVOKABLE void pasteClipAt(int trackIndex, double startTimeSeconds);
    Q_INVOKABLE void deleteClip(int trackIndex, int clipIndex);
    Q_INVOKABLE QVariantMap getClipDetails(int trackIndex, int clipIndex) const;

private Q_SLOTS:
    void refreshTracks();
    void onEditChanged();

private:
    void valueTreeChanged() override;
    bool isTrackArmed(tracktion::engine::AudioTrack* track) const;

    EngineController* m_controller = nullptr;
    tracktion::engine::Edit* m_boundEdit = nullptr;
    std::vector<tracktion::engine::AudioTrack*> tracks;
    std::vector<std::unique_ptr<ClipListModel>> clipModels;

    struct TrackMeterState {
        tracktion::engine::LevelMeasurer::Client client;
        juce::WeakReference<tracktion::engine::LevelMeasurer> measurer;
        float smoothedLevel = 0.0f;
    };
    mutable std::vector<std::unique_ptr<TrackMeterState>> meterStates;

    juce::ValueTree copiedClipState;
    QTimer meterTimer;
};
