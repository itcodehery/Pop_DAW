#include "ClipListModel.h"
#include <QColor>

ClipListModel::ClipListModel(tracktion::engine::Track* trk, QObject* parent)
    : QAbstractListModel(parent), track(trk)
{
    if (track) {
        track->state.addListener(this);
        refreshClips();
    }
}

ClipListModel::~ClipListModel()
{
    if (track)
        track->state.removeListener(this);
}

int ClipListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(clips.size());
}

QVariant ClipListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(clips.size()))
        return QVariant();

    auto* clip = clips[static_cast<size_t>(index.row())];
    auto* audioClip = dynamic_cast<tracktion::engine::AudioClipBase*>(clip);

    switch (role) {
        case NameRole:
            return QString::fromStdString(clip->getName().toStdString());
            
        case StartTimeRole:
            return clip->getPosition().time.getStart().inSeconds();
            
        case LengthRole:
            return clip->getPosition().time.getLength().inSeconds();

        case OffsetRole:
            return clip->getPosition().offset.inSeconds();

        case GainDbRole:
            return audioClip ? audioClip->getGainDB() : 0.0f;

        case FadeInRole:
            return audioClip ? audioClip->getFadeIn().inSeconds() : 0.0;

        case FadeOutRole:
            return audioClip ? audioClip->getFadeOut().inSeconds() : 0.0;

        case IsReversedRole:
            return audioClip ? audioClip->getIsReversed() : false;

        case PitchSemitonesRole:
            return audioClip ? audioClip->getPitchChange() : 0.0f;

        case SpeedRatioRole:
            return clip->getSpeedRatio();
            
        case ColorRole: {
            auto c = clip->getColour();
            if (c.isTransparent()) {
                if (track) c = track->getColour();
            }
            if (c.isTransparent())
                return QColor("#74A8E8");
            return QColor(c.getRed(), c.getGreen(), c.getBlue());
        }
            
        case IsMidiRole:
            return clip->type == tracktion::engine::TrackItem::Type::midi;
    }

    return QVariant();
}

QHash<int, QByteArray> ClipListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "clipName";
    roles[StartTimeRole] = "clipStartTime";
    roles[LengthRole] = "clipLength";
    roles[OffsetRole] = "clipOffset";
    roles[GainDbRole] = "clipGainDb";
    roles[FadeInRole] = "clipFadeIn";
    roles[FadeOutRole] = "clipFadeOut";
    roles[IsReversedRole] = "clipIsReversed";
    roles[PitchSemitonesRole] = "clipPitch";
    roles[SpeedRatioRole] = "clipSpeed";
    roles[ColorRole] = "clipColor";
    roles[IsMidiRole] = "isMidi";
    return roles;
}

void ClipListModel::valueTreeChanged()
{
    QMetaObject::invokeMethod(this, "refreshClips", Qt::QueuedConnection);
}

void ClipListModel::refreshClips()
{
    beginResetModel();
    clips.clear();
    if (track) {
        if (auto* at = dynamic_cast<tracktion::engine::ClipTrack*>(track)) {
            for (auto* c : at->getClips())
                clips.push_back(c);
        }
    }
    endResetModel();
}
