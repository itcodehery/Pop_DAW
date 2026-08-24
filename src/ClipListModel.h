#pragma once

#include <QAbstractListModel>
#include <tracktion_engine/tracktion_engine.h>

class ClipListModel : public QAbstractListModel, private tracktion::engine::ValueTreeAllEventListener
{
    Q_OBJECT

public:
    enum ClipRoles {
        NameRole = Qt::UserRole + 1,
        StartTimeRole,
        LengthRole,
        ColorRole,
        IsMidiRole
    };

    explicit ClipListModel(tracktion::engine::Track* track, QObject* parent = nullptr);
    ~ClipListModel() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void valueTreeChanged() override;
    void refreshClips();

    tracktion::engine::Track* track;
    std::vector<tracktion::engine::Clip*> clips;
};
