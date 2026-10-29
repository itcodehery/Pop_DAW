#include "TrackListModel.h"
#include "ClipListModel.h"
#include "EngineController.h"
#include <algorithm>
#include <cmath>

static const std::vector<juce::Colour> kPastelTrackColours = {
    juce::Colour(0xff98E87B), // Green
    juce::Colour(0xff74A8E8), // Blue
    juce::Colour(0xffE5C07B), // Yellow/Orange
    juce::Colour(0xffE06C75), // Coral Red
    juce::Colour(0xffC678DD), // Purple
    juce::Colour(0xff56B6C2)  // Cyan
};

TrackListModel::TrackListModel(EngineController* controller, QObject* parent)
    : QAbstractListModel(parent), m_controller(controller)
{
    if (m_controller) {
        connect(m_controller, &EngineController::editChanged, this, &TrackListModel::onEditChanged);
        onEditChanged();
    }
    
    connect(&meterTimer, &QTimer::timeout, this, [this]() {
        if (!tracks.empty()) {
            Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {LevelRole});
        }
    });
    meterTimer.start(30); // ~30 FPS
}

TrackListModel::~TrackListModel()
{
    for (auto& ms : meterStates) {
        if (ms && ms->measurer != nullptr)
            ms->measurer->removeClient(ms->client);
    }
    if (m_boundEdit)
        m_boundEdit->state.removeListener(this);
}

void TrackListModel::onEditChanged()
{
    for (auto& ms : meterStates) {
        if (ms && ms->measurer != nullptr)
            ms->measurer->removeClient(ms->client);
    }
    meterStates.clear();

    if (m_boundEdit)
        m_boundEdit->state.removeListener(this);

    m_boundEdit = m_controller ? m_controller->getEdit() : nullptr;

    if (m_boundEdit)
        m_boundEdit->state.addListener(this);

    refreshTracks();
}

int TrackListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(tracks.size());
}

bool TrackListModel::isTrackArmed(tracktion::engine::AudioTrack* track) const
{
    if (!m_boundEdit || !track) return false;
    for (auto* inst : m_boundEdit->getAllInputDevices()) {
        if (tracktion::engine::isOnTargetTrack(*inst, *track, 0) && inst->isRecordingEnabled(track->itemID))
            return true;
    }
    return false;
}

QVariant TrackListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(tracks.size()))
        return QVariant();

    size_t row = static_cast<size_t>(index.row());
    auto* track = tracks[row];

    switch (role) {
        case NameRole:
            return QString::fromStdString(track->getName().toStdString());
            
        case ColorRole: {
            auto c = track->getColour();
            if (c.isTransparent()) {
                c = kPastelTrackColours[row % kPastelTrackColours.size()];
            }
            return QColor(c.getRed(), c.getGreen(), c.getBlue());
        }
             
        case MuteRole:
            return track->isMuted(false);
            
        case SoloRole:
            return track->isSolo(false);

        case ArmedRole:
            return isTrackArmed(track);
            
        case VolumeRole:
            if (auto volPlugin = track->getVolumePlugin())
                return volPlugin->getVolumeDb();
            return 0.0f;

        case PanRole:
            if (auto volPlugin = track->getVolumePlugin())
                return volPlugin->getPan();
            return 0.0f;
            
        case LevelRole: {
            if (row < meterStates.size() && meterStates[row]) {
                auto& ms = *meterStates[row];
                if (auto* meterPlugin = track->getLevelMeterPlugin()) {
                    if (&meterPlugin->measurer != ms.measurer.get()) {
                        if (ms.measurer != nullptr)
                            ms.measurer->removeClient(ms.client);
                        ms.measurer = &meterPlugin->measurer;
                        ms.measurer->addClient(ms.client);
                    }
                    float dbL = ms.client.getAndClearAudioLevel(0).dB;
                    float dbR = ms.client.getAndClearAudioLevel(1).dB;
                    float maxDb = std::max(dbL, dbR);
                    float target = std::clamp((maxDb + 60.0f) / 66.0f, 0.0f, 1.0f);
                    if (track->isMuted(true)) target = 0.0f;
                    ms.smoothedLevel = ms.smoothedLevel * 0.6f + target * 0.4f;
                    return ms.smoothedLevel;
                }
            }
            return 0.0f;
        }

        case ClipsRole:
            return QVariant::fromValue(clipModels[row].get());
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
        case PanRole: {
            if (auto volPlugin = track->getVolumePlugin()) {
                volPlugin->setPan(std::clamp(value.toFloat(), -1.0f, 1.0f));
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
    roles[ArmedRole] = "isArmed";
    roles[VolumeRole] = "volumeDb";
    roles[PanRole] = "pan";
    roles[LevelRole] = "levelMeter";
    roles[ClipsRole] = "clipModel";
    return roles;
}

void TrackListModel::addTrack()
{
    if (!m_boundEdit) return;
    int nextCount = tracktion::engine::getAudioTracks(*m_boundEdit).size() + 1;
    m_boundEdit->ensureNumberOfAudioTracks(nextCount);
    auto audioTracks = tracktion::engine::getAudioTracks(*m_boundEdit);
    if (!audioTracks.isEmpty()) {
        auto* newest = audioTracks.getLast();
        newest->setColour(kPastelTrackColours[static_cast<size_t>(nextCount - 1) % kPastelTrackColours.size()]);
    }
}

void TrackListModel::removeTrack(int index)
{
    if (!m_boundEdit || index < 0 || index >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(index)];
    m_boundEdit->deleteTrack(track);
}

void TrackListModel::renameTrack(int index, const QString& newName)
{
    if (!m_boundEdit || index < 0 || index >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(index)];
    track->setName(juce::String(newName.toStdString()));
    Q_EMIT dataChanged(this->index(index, 0), this->index(index, 0), {NameRole});
}

void TrackListModel::toggleTrackMute(int index)
{
    if (!m_boundEdit || index < 0 || index >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(index)];
    track->setMute(!track->isMuted(false));
    Q_EMIT dataChanged(this->index(index, 0), this->index(index, 0), {MuteRole});
}

void TrackListModel::toggleTrackSolo(int index)
{
    if (!m_boundEdit || index < 0 || index >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(index)];
    track->setSolo(!track->isSolo(false));
    Q_EMIT dataChanged(this->index(index, 0), this->index(index, 0), {SoloRole});
}

void TrackListModel::toggleTrackArm(int index)
{
    if (!m_boundEdit || index < 0 || index >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(index)];
    bool currentlyArmed = isTrackArmed(track);

    m_boundEdit->getTransport().ensureContextAllocated();

    for (auto* inst : m_boundEdit->getAllInputDevices()) {
        if (inst->getInputDevice().getDeviceType() == tracktion::engine::InputDevice::waveDevice) {
            if (!currentlyArmed) {
                [[maybe_unused]] auto res = inst->setTarget(track->itemID, true, &m_boundEdit->getUndoManager(), 0);
                inst->setRecordingEnabled(track->itemID, true);
            } else if (tracktion::engine::isOnTargetTrack(*inst, *track, 0)) {
                inst->setRecordingEnabled(track->itemID, false);
            }
            break;
        }
    }
    m_boundEdit->restartPlayback();
    Q_EMIT dataChanged(this->index(index, 0), this->index(index, 0), {ArmedRole});
}

void TrackListModel::setTrackVolume(int index, float db)
{
    if (!m_boundEdit || index < 0 || index >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(index)];
    if (auto volPlugin = track->getVolumePlugin()) {
        volPlugin->setVolumeDb(std::clamp(db, -60.0f, 12.0f));
        Q_EMIT dataChanged(this->index(index, 0), this->index(index, 0), {VolumeRole});
    }
}

void TrackListModel::setTrackPan(int index, float pan)
{
    if (!m_boundEdit || index < 0 || index >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(index)];
    if (auto volPlugin = track->getVolumePlugin()) {
        volPlugin->setPan(std::clamp(pan, -1.0f, 1.0f));
        Q_EMIT dataChanged(this->index(index, 0), this->index(index, 0), {PanRole});
    }
}

void TrackListModel::moveClip(int trackIndex, int clipIndex, double newStartSeconds)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            auto* clip = clips[clipIndex];
            auto pos = clip->getPosition();
            double start = std::max(0.0, newStartSeconds);
            pos.time = { tracktion::TimePosition::fromSeconds(start), pos.time.getLength() };
            clip->setPosition(pos);
        }
    }
}

void TrackListModel::trimClipLeft(int trackIndex, int clipIndex, double newStartSeconds)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            auto* clip = clips[clipIndex];
            auto pos = clip->getPosition();
            double oldStart = pos.time.getStart().inSeconds();
            double oldEnd = pos.time.getEnd().inSeconds();
            double oldOffset = pos.offset.inSeconds();

            // Clamp so offset doesn't go below 0 and length stays >= 0.05s
            double minStart = std::max(0.0, oldStart - oldOffset);
            double clampedStart = std::clamp(newStartSeconds, minStart, oldEnd - 0.05);
            double delta = clampedStart - oldStart;

            pos.time = { tracktion::TimePosition::fromSeconds(clampedStart),
                         tracktion::TimePosition::fromSeconds(oldEnd) };
            pos.offset = tracktion::TimeDuration::fromSeconds(std::max(0.0, oldOffset + delta));
            clip->setPosition(pos);
        }
    }
}

void TrackListModel::trimClipRight(int trackIndex, int clipIndex, double newLengthSeconds)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            auto* clip = clips[clipIndex];
            double clampedLen = std::max(0.05, newLengthSeconds);
            if (auto* waveClip = dynamic_cast<tracktion::engine::WaveAudioClip*>(clip)) {
                double srcLen = waveClip->getSourceLength().inSeconds();
                double offset = waveClip->getPosition().offset.inSeconds();
                if (srcLen > 0.0 && !waveClip->isLooping()) {
                    clampedLen = std::min(clampedLen, std::max(0.05, srcLen - offset));
                }
            }
            clip->setLength(tracktion::TimeDuration::fromSeconds(clampedLen), true);
        }
    }
}

void TrackListModel::setClipFadeIn(int trackIndex, int clipIndex, double fadeInSeconds)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            if (auto* acb = dynamic_cast<tracktion::engine::AudioClipBase*>(clips[clipIndex])) {
                double len = acb->getPosition().time.getLength().inSeconds();
                double clamped = std::clamp(fadeInSeconds, 0.0, len);
                acb->setFadeIn(tracktion::TimeDuration::fromSeconds(clamped));
            }
        }
    }
}

void TrackListModel::setClipFadeOut(int trackIndex, int clipIndex, double fadeOutSeconds)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            if (auto* acb = dynamic_cast<tracktion::engine::AudioClipBase*>(clips[clipIndex])) {
                double len = acb->getPosition().time.getLength().inSeconds();
                double clamped = std::clamp(fadeOutSeconds, 0.0, len);
                acb->setFadeOut(tracktion::TimeDuration::fromSeconds(clamped));
            }
        }
    }
}

void TrackListModel::setClipGainDb(int trackIndex, int clipIndex, float gainDb)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            if (auto* acb = dynamic_cast<tracktion::engine::AudioClipBase*>(clips[clipIndex])) {
                acb->setGainDB(std::clamp(gainDb, -60.0f, 24.0f));
            }
        }
    }
}

void TrackListModel::toggleClipReverse(int trackIndex, int clipIndex)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            if (auto* acb = dynamic_cast<tracktion::engine::AudioClipBase*>(clips[clipIndex])) {
                acb->setIsReversed(!acb->getIsReversed());
            }
        }
    }
}

void TrackListModel::setClipPitch(int trackIndex, int clipIndex, float semitones)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            if (auto* acb = dynamic_cast<tracktion::engine::AudioClipBase*>(clips[clipIndex])) {
                acb->setPitchChange(std::clamp(semitones, -24.0f, 24.0f));
            }
        }
    }
}

void TrackListModel::setClipSpeed(int trackIndex, int clipIndex, double ratio)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            clips[clipIndex]->setSpeedRatio(std::clamp(ratio, 0.25, 4.0));
        }
    }
}

void TrackListModel::normalizeClip(int trackIndex, int clipIndex)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            if (auto* waveClip = dynamic_cast<tracktion::engine::WaveAudioClip*>(clips[clipIndex])) {
                auto audioFile = waveClip->getAudioFile();
                if (!audioFile.isValid()) return;
                auto& fm = m_boundEdit->engine.getAudioFileFormatManager().readFormatManager;
                std::unique_ptr<juce::AudioFormatReader> reader(fm.createReaderFor(audioFile.getFile()));
                if (!reader) return;

                double offsetSec = waveClip->getPosition().offset.inSeconds();
                double lenSec = waveClip->getPosition().time.getLength().inSeconds();
                juce::int64 startSample = static_cast<juce::int64>(offsetSec * reader->sampleRate);
                juce::int64 numSamples = static_cast<juce::int64>(lenSec * reader->sampleRate);
                if (numSamples <= 0) return;

                float lowL = 0, highL = 0, lowR = 0, highR = 0;
                reader->readMaxLevels(startSample, numSamples, lowL, highL, lowR, highR);
                float maxPeak = std::max({ std::abs(lowL), std::abs(highL), std::abs(lowR), std::abs(highR) });
                if (maxPeak > 0.0001f) {
                    float targetDb = -tracktion::engine::gainToDb(maxPeak);
                    waveClip->setGainDB(std::clamp(targetDb, -60.0f, 24.0f));
                }
            }
        }
    }
}

void TrackListModel::splitClip(int trackIndex, int clipIndex, double splitTimeSeconds)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
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

void TrackListModel::duplicateClip(int trackIndex, int clipIndex)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            auto* src = clips[clipIndex];
            auto newState = src->state.createCopy();
            m_boundEdit->createNewItemID().writeID(newState, nullptr);
            double endPos = src->getPosition().time.getEnd().inSeconds();
            newState.setProperty(tracktion::engine::IDs::start, endPos, nullptr);
            clipTrack->state.addChild(newState, -1, &m_boundEdit->getUndoManager());
        }
    }
}

void TrackListModel::copyClip(int trackIndex, int clipIndex)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            copiedClipState = clips[clipIndex]->state.createCopy();
        }
    }
}

void TrackListModel::pasteClipAt(int trackIndex, double startTimeSeconds)
{
    if (!m_boundEdit || !copiedClipState.isValid() || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size()))
        return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto newState = copiedClipState.createCopy();
        m_boundEdit->createNewItemID().writeID(newState, nullptr);
        newState.setProperty(tracktion::engine::IDs::start, std::max(0.0, startTimeSeconds), nullptr);
        clipTrack->state.addChild(newState, -1, &m_boundEdit->getUndoManager());
    }
}

void TrackListModel::deleteClip(int trackIndex, int clipIndex)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            auto* clip = clips[clipIndex];
            clip->removeFromParent();
        }
    }
}

QVariantMap TrackListModel::getClipDetails(int trackIndex, int clipIndex) const
{
    QVariantMap map;
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return map;
    auto* track = tracks[static_cast<size_t>(trackIndex)];
    if (auto* clipTrack = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
        auto clips = clipTrack->getClips();
        if (clipIndex >= 0 && clipIndex < clips.size()) {
            auto* clip = clips[clipIndex];
            auto* acb = dynamic_cast<tracktion::engine::AudioClipBase*>(clip);
            map["name"] = QString::fromStdString(clip->getName().toStdString());
            map["start"] = clip->getPosition().time.getStart().inSeconds();
            map["length"] = clip->getPosition().time.getLength().inSeconds();
            map["offset"] = clip->getPosition().offset.inSeconds();
            map["gainDb"] = acb ? acb->getGainDB() : 0.0f;
            map["fadeIn"] = acb ? acb->getFadeIn().inSeconds() : 0.0;
            map["fadeOut"] = acb ? acb->getFadeOut().inSeconds() : 0.0;
            map["reversed"] = acb ? acb->getIsReversed() : false;
            map["pitch"] = acb ? acb->getPitchChange() : 0.0f;
            map["speed"] = clip->getSpeedRatio();
        }
    }
    return map;
}

void TrackListModel::addMidiClip(int trackIndex, double startTime, double length)
{
    if (!m_boundEdit || trackIndex < 0 || trackIndex >= static_cast<int>(tracks.size())) return;
    
    if (auto* at = dynamic_cast<tracktion::engine::AudioTrack*>(tracks[static_cast<size_t>(trackIndex)])) {
        tracktion::TimeRange range(tracktion::TimePosition::fromSeconds(startTime), 
                                   tracktion::TimePosition::fromSeconds(startTime + length));
        at->insertMIDIClip("MIDI Clip", range, nullptr);
    }
}

void TrackListModel::valueTreeChanged()
{
    // Only rebuild the whole track list if the number of audio tracks actually changed
    if (!m_boundEdit) return;
    auto currentTracks = tracktion::engine::getAudioTracks(*m_boundEdit);
    bool structureChanged = (currentTracks.size() != static_cast<int>(tracks.size()));
    if (!structureChanged) {
        for (int i = 0; i < currentTracks.size(); ++i) {
            if (currentTracks[i] != tracks[static_cast<size_t>(i)]) {
                structureChanged = true;
                break;
            }
        }
    }
    if (structureChanged) {
        QMetaObject::invokeMethod(this, "refreshTracks", Qt::QueuedConnection);
    }
}

void TrackListModel::refreshTracks()
{
    beginResetModel();
    for (auto& ms : meterStates) {
        if (ms && ms->measurer != nullptr)
            ms->measurer->removeClient(ms->client);
    }
    tracks.clear();
    clipModels.clear();
    meterStates.clear();
    if (m_boundEdit) {
        size_t idx = 0;
        for (auto* t : tracktion::engine::getAudioTracks(*m_boundEdit)) {
            if (t->getColour().isTransparent()) {
                t->setColour(kPastelTrackColours[idx % kPastelTrackColours.size()]);
            }
            tracks.push_back(t);
            clipModels.push_back(std::make_unique<ClipListModel>(t, this));
            meterStates.push_back(std::make_unique<TrackMeterState>());
            ++idx;
        }
    }
    endResetModel();
}
