#pragma once
#include <QQuickPaintedItem>
#include <QPainter>
#include <tracktion_engine/tracktion_engine.h>

class EngineController;

class WaveformItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(EngineController* engineController READ engineController WRITE setEngineController NOTIFY engineControllerChanged)
    Q_PROPERTY(int trackIndex READ trackIndex WRITE setTrackIndex NOTIFY trackIndexChanged)
    Q_PROPERTY(int clipIndex READ clipIndex WRITE setClipIndex NOTIFY clipIndexChanged)
public:
    WaveformItem(QQuickItem* parent = nullptr);
    void paint(QPainter* painter) override;

    EngineController* engineController() const { return m_controller; }
    void setEngineController(EngineController* c) { m_controller = c; Q_EMIT engineControllerChanged(); update(); }

    int trackIndex() const { return m_trackIndex; }
    void setTrackIndex(int i) { m_trackIndex = i; Q_EMIT trackIndexChanged(); update(); }

    int clipIndex() const { return m_clipIndex; }
    void setClipIndex(int i) { m_clipIndex = i; Q_EMIT clipIndexChanged(); update(); }

Q_SIGNALS:
    void engineControllerChanged();
    void trackIndexChanged();
    void clipIndexChanged();

private:
    EngineController* m_controller = nullptr;
    int m_trackIndex = -1;
    int m_clipIndex = -1;
};
