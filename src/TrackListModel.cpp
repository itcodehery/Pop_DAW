#include "TrackListModel.h"
#include "ClipListModel.h"

TrackListModel::TrackListModel(tracktion::engine::Edit* ed, QObject* parent)
    : QAbstractListModel(parent), edit(ed)
{
    if (edit) {
        edit->state.addListener(this);
        refreshTracks();
    }
    
    // Timer to update track level meters
    connect(&meterTimer, &QTimer::timeout, this, [this]() {
        if (!tracks.empty()) {
            Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {LevelRole});
        }
    });
    meterTimer.start(30); // ~30 FPS
}

TrackListModel::~TrackListModel()
{
    if (edit)
        edit->state.removeListener(this);
}

int TrackListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(tracks.size());
}

QVariant TrackListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(tracks.size()))
        return QVariant();

    auto* track = tracks[static_cast<size_t>(index.row())];

    switch (role) {
        case NameRole:
            return QString::fromStdString(track->getName().toStdString());
            
        case ColorRole: {
            auto c = track->getColour();
            return QColor(c.getRed(), c.getGreen(), c.getBlue());
        }
            
        case MuteRole:
            return track->isMuted(false);
            
        case SoloRole:
            return track->isSolo(false);
            
        case VolumeRole:
            if (auto volPlugin = track->getVolumePlugin())
                return volPlugin->getVolumeDb();
            return 0.0f;
            
        case LevelRole: {
            // Placeholder: getting real levels requires the level measurer logic
            // For now we'll just return a mock random level between 0 and 1 if playing
            if (edit && edit->getTransport().isPlaying())
                return (std::rand() % 100) / 100.0f;
            return 0.0f;
        }

        case ClipsRole:
            return QVariant::fromValue(clipModels[static_cast<size_t>(index.row())].get());
    }

    return QVariant();
}

bool TrackListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() >= static_cast<int>(tracks.size()))
        return false;

    auto* track = tracks[static_cast<size_t>(index.row())];
    bool changed = false;

    switch (role) {
        case NameRole: {
            QString newName = value.toString();
            if (newName.toStdString() != track->getName().toStdString()) {
                track->setName(juce::String(newName.toStdString()));
                changed = true;
            }
            break;
        }
        case MuteRole: {
            track->setMute(value.toBool());
            changed = true;
            break;
        }
        case SoloRole: {
            track->setSolo(value.toBool());
            changed = true;
            break;
        }
        case VolumeRole: {
            if (auto volPlugin = track->getVolumePlugin()) {
                volPlugin->setVolumeDb(value.toFloat());
                changed = true;
            }
            break;
        }
    }

    if (changed) {
        Q_EMIT dataChanged(index, index, {role});
        return true;
    }
    return false;
}

QHash<int, QByteArray> TrackListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "trackName";
    roles[ColorRole] = "trackColor";
    roles[MuteRole] = "isMuted";
    roles[SoloRole] = "isSoloed";
    roles[VolumeRole] = "volumeDb";
    roles[LevelRole] = "levelMeter";
    roles[ClipsRole] = "clipModel";
    return roles;
}

void TrackListModel::addTrack()
{
    if (!edit) return;
    edit->ensureNumberOfAudioTracks(tracktion::engine::getAudioTracks(*edit).size() + 1);
}

void TrackListModel::removeTrack(int index)
{
    if (!edit || index < 0 || index >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(index)];
    edit->deleteTrack(track);
}

void TrackListModel::toggleTrackMute(int index)
{
    if (!edit || index < 0 || index >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(index)];
    track->setMute(!track->isMuted(false));
    Q_EMIT dataChanged(this->index(index, 0), this->index(index, 0), {MuteRole});
}

void TrackListModel::toggleTrackSolo(int index)
{
    if (!edit || index < 0 || index >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(index)];
    track->setSolo(!track->isSolo(false));
    Q_EMIT dataChanged(this->index(index, 0), this->index(index, 0), {SoloRole});
}

void TrackListModel::setTrackVolume(int index, float db)
{
    if (!edit || index < 0 || index >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(index)];
    
    // In tracktion, volume plugin is automatically added to tracks, or we set the volume on the track's volume plugin.
    auto volPlugin = track->getVolumePlugin();
    if (volPlugin) {
        volPlugin->setVolumeDb(db);
        Q_EMIT dataChanged(this->index(index, 0), this->index(index, 0), {VolumeRole});
    }
}

void TrackListModel::moveClip(int trackIndex, int clipIndex, double newStartSeconds)
{
    if (!edit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            auto* clip = clips[clipIndex];
            auto pos = clip->getPosition();
            pos.time = { tracktion::TimePosition::fromSeconds(newStartSeconds), pos.time.getLength() };
            clip->setPosition(pos);
        }
    }
}

void TrackListModel::splitClip(int trackIndex, int clipIndex, double splitTimeSeconds)
{
    if (!edit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            auto* clip = clips[clipIndex];
            tracktion::engine::SelectableList list;
            list.add(clip);
            tracktion::engine::splitClips(list, tracktion::TimePosition::fromSeconds(splitTimeSeconds));
        }
    }
}

void TrackListModel::deleteClip(int trackIndex, int clipIndex)
{
    if (!edit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            auto* clip = clips[clipIndex];
            clip->removeFromParent();
        }
    }
}

void TrackListModel::addMidiClip(int trackIndex, double startTime, double length)
{
    if (!edit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    
    if (auto* at = dynamic_cast<tracktion::engine::AudioTrack*>(tracks[static_cast<size_t>(trackIndex)])) {
        tracktion::TimeRange range(tracktion::TimePosition::fromSeconds(startTime), 
                                   tracktion::TimePosition::fromSeconds(startTime + length));
        at->insertMIDIClip("MIDI Clip", range, nullptr);
    }
}

void TrackListModel::valueTreeChanged()
{
    // A blunt hammer approach: rebuild track list if the tree changes
    // In a full implementation, you'd listen to specific child added/removed events
    QMetaObject::invokeMethod(this, "refreshTracks", Qt::QueuedConnection);
}

void TrackListModel::refreshTracks()
{
    beginResetModel();
    tracks.clear();
    clipModels.clear();
    if (edit) {
        for (auto* t : tracktion::engine::getAudioTracks(*edit)) {
            tracks.push_back(t);
            clipModels.push_back(std::make_unique<ClipListModel>(t, this));
        }
    }
    endResetModel();
}
