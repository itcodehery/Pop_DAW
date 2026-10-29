#pragma once
#include <QQuickPaintedItem>
#include <QPainter>
#include <QPolygonF>
#include <cmath>
#include <tracktion_engine/tracktion_engine.h>

class EngineController;

class WaveformItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(EngineController* engineController READ engineController WRITE setEngineController NOTIFY engineControllerChanged)
    Q_PROPERTY(int trackIndex READ trackIndex WRITE setTrackIndex NOTIFY trackIndexChanged)
    Q_PROPERTY(int clipIndex READ clipIndex WRITE setClipIndex NOTIFY clipIndexChanged)
    Q_PROPERTY(double clipOffset READ clipOffset WRITE setClipOffset NOTIFY clipParamsChanged)
    Q_PROPERTY(double clipLength READ clipLength WRITE setClipLength NOTIFY clipParamsChanged)
    Q_PROPERTY(float clipGainDb READ clipGainDb WRITE setClipGainDb NOTIFY clipParamsChanged)
    Q_PROPERTY(double fadeIn READ fadeIn WRITE setFadeIn NOTIFY clipParamsChanged)
    Q_PROPERTY(double fadeOut READ fadeOut WRITE setFadeOut NOTIFY clipParamsChanged)
    Q_PROPERTY(bool isReversed READ isReversed WRITE setIsReversed NOTIFY clipParamsChanged)
public:
    WaveformItem(QQuickItem* parent = nullptr);
    void paint(QPainter* painter) override;

    EngineController* engineController() const { return m_controller; }
    void setEngineController(EngineController* c) { m_controller = c; Q_EMIT engineControllerChanged(); update(); }

    int trackIndex() const { return m_trackIndex; }
    void setTrackIndex(int i) { m_trackIndex = i; Q_EMIT trackIndexChanged(); update(); }

    int clipIndex() const { return m_clipIndex; }
    void setClipIndex(int i) { m_clipIndex = i; Q_EMIT clipIndexChanged(); update(); }

    double clipOffset() const { return m_clipOffset; }
    void setClipOffset(double v) { if (std::abs(m_clipOffset - v) > 1e-6) { m_clipOffset = v; Q_EMIT clipParamsChanged(); update(); } }

    double clipLength() const { return m_clipLength; }
    void setClipLength(double v) { if (std::abs(m_clipLength - v) > 1e-6) { m_clipLength = v; Q_EMIT clipParamsChanged(); update(); } }

    float clipGainDb() const { return m_clipGainDb; }
    void setClipGainDb(float v) { if (std::abs(m_clipGainDb - v) > 1e-4f) { m_clipGainDb = v; Q_EMIT clipParamsChanged(); update(); } }

    double fadeIn() const { return m_fadeIn; }
    void setFadeIn(double v) { if (std::abs(m_fadeIn - v) > 1e-6) { m_fadeIn = v; Q_EMIT clipParamsChanged(); update(); } }

    double fadeOut() const { return m_fadeOut; }
    void setFadeOut(double v) { if (std::abs(m_fadeOut - v) > 1e-6) { m_fadeOut = v; Q_EMIT clipParamsChanged(); update(); } }

    bool isReversed() const { return m_isReversed; }
    void setIsReversed(bool v) { if (m_isReversed != v) { m_isReversed = v; Q_EMIT clipParamsChanged(); update(); } }

Q_SIGNALS:
    void engineControllerChanged();
    void trackIndexChanged();
    void clipIndexChanged();
    void clipParamsChanged();

private:
    EngineController* m_controller = nullptr;
    int m_trackIndex = -1;
    int m_clipIndex = -1;
    double m_clipOffset = 0.0;
    double m_clipLength = 0.0;
    float m_clipGainDb = 0.0f;
    double m_fadeIn = 0.0;
    double m_fadeOut = 0.0;
    bool m_isReversed = false;
};
