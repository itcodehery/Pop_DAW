#pragma once

#include <QAbstractListModel>
#include <QColor>
#include <QTimer>
#include <tracktion_engine/tracktion_engine.h>

class TrackListModel : public QAbstractListModel, private tracktion::engine::ValueTreeAllEventListener
{
    Q_OBJECT

public:
    enum TrackRoles {
        NameRole = Qt::UserRole + 1,
        ColorRole,
        MuteRole,
        SoloRole,
        VolumeRole,
        LevelRole,
        ClipsRole
    };

    explicit TrackListModel(tracktion::engine::Edit* edit, QObject* parent = nullptr);
    ~TrackListModel() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addTrack();
    Q_INVOKABLE void removeTrack(int index);
    Q_INVOKABLE void moveClip(int trackIndex, int clipIndex, double newStartSeconds);
    Q_INVOKABLE void addMidiClip(int trackIndex, double startTime, double length);

private Q_SLOTS:
    void refreshTracks();

private:
    void valueTreeChanged() override;

    tracktion::engine::Edit* edit;
    std::vector<tracktion::engine::AudioTrack*> tracks;
    std::vector<std::unique_ptr<class ClipListModel>> clipModels;
    
    QTimer meterTimer; // For updating level meters
};
