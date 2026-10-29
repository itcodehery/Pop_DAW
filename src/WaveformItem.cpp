#include "WaveformItem.h"
#include "EngineController.h"
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <cmath>
#include <algorithm>

namespace {
struct CachedWaveformEnvelope {
    double sampleRate = 44100.0;
    double durationSeconds = 0.0;
    int binsPerSecond = 200; // 5ms resolution
    std::vector<float> minPeaks;
    std::vector<float> maxPeaks;
};

static QMutex g_cacheMutex;
static QMap<QString, CachedWaveformEnvelope> g_waveformCache;

const CachedWaveformEnvelope* getOrBuildEnvelope(tracktion::engine::Engine& engine, const juce::File& file)
{
    QString key = QString::fromStdString(file.getFullPathName().toStdString());
    {
        QMutexLocker lock(&g_cacheMutex);
        auto it = g_waveformCache.find(key);
        if (it != g_waveformCache.end())
            return &it.value();
    }

    auto& fm = engine.getAudioFileFormatManager().readFormatManager;
    std::unique_ptr<juce::AudioFormatReader> reader(fm.createReaderFor(file));
    if (!reader || reader->lengthInSamples <= 0 || reader->sampleRate <= 0)
        return nullptr;

    CachedWaveformEnvelope env;
    env.sampleRate = reader->sampleRate;
    env.durationSeconds = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
    env.binsPerSecond = 200;
    int totalBins = std::max(1, static_cast<int>(std::ceil(env.durationSeconds * env.binsPerSecond)));
    env.minPeaks.resize(static_cast<size_t>(totalBins), 0.0f);
    env.maxPeaks.resize(static_cast<size_t>(totalBins), 0.0f);

    juce::int64 samplesPerBin = std::max<juce::int64>(1, reader->lengthInSamples / totalBins);
    for (int b = 0; b < totalBins; ++b) {
        juce::int64 startSample = static_cast<juce::int64>(b) * samplesPerBin;
        juce::int64 count = std::min<juce::int64>(samplesPerBin, reader->lengthInSamples - startSample);
        if (count <= 0) break;
        float lowL = 0.0f, highL = 0.0f, lowR = 0.0f, highR = 0.0f;
        reader->readMaxLevels(startSample, count, lowL, highL, lowR, highR);
        env.minPeaks[static_cast<size_t>(b)] = std::min(lowL, lowR);
        env.maxPeaks[static_cast<size_t>(b)] = std::max(highL, highR);
    }

    QMutexLocker lock(&g_cacheMutex);
    auto it = g_waveformCache.insert(key, std::move(env));
    return &it.value();
}
} // namespace

WaveformItem::WaveformItem(QQuickItem* parent) : QQuickPaintedItem(parent) {
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
    connect(this, &QQuickItem::widthChanged, this, [this]() { update(); });
    connect(this, &QQuickItem::heightChanged, this, [this]() { update(); });
}

void WaveformItem::paint(QPainter* painter) {
    if (!m_controller || m_trackIndex < 0 || m_clipIndex < 0) return;
    auto* edit = m_controller->getEdit();
    if (!edit) return;
    
    auto tracks = tracktion::engine::getAudioTracks(*edit);
    if (m_trackIndex >= tracks.size()) return;
    
    auto* track = tracks[m_trackIndex];
    auto clips = track->getClips();
    if (m_clipIndex >= clips.size()) return;
    
    auto* waveClip = dynamic_cast<tracktion::engine::WaveAudioClip*>(clips[m_clipIndex]);
    if (!waveClip) return;
    
    int w = static_cast<int>(boundingRect().width());
    int h = static_cast<int>(boundingRect().height());
    if (w <= 0 || h <= 0) return;
    int midY = h / 2;

    auto duration = waveClip->getPosition().time.getLength().inSeconds();
    if (duration <= 0.0) return;

    auto audioFile = waveClip->getAudioFile();
    if (!audioFile.isValid()) return;

    const auto* env = getOrBuildEnvelope(edit->engine, audioFile.getFile());
    if (!env || env->minPeaks.empty()) {
        painter->setPen(QPen(QColor(17, 17, 17, 150), 1.0));
        painter->drawLine(0, midY, w, midY);
        return;
    }

    double offsetSec = waveClip->getPosition().offset.inSeconds();
    double speedRatio = std::max(0.05, waveClip->getSpeedRatio());
    float gainLinear = tracktion::engine::dbToGain(waveClip->getGainDB());
    double fadeInSec = waveClip->getFadeIn().inSeconds();
    double fadeOutSec = waveClip->getFadeOut().inSeconds();
    bool rev = waveClip->getIsReversed();

    painter->setRenderHint(QPainter::Antialiasing, false);

    // Zero-crossing center line
    painter->setPen(QPen(QColor(0, 0, 0, 60), 1.0));
    painter->drawLine(0, midY, w, midY);

    painter->setPen(QPen(QColor(18, 18, 20, 210), 1.0));

    int totalBins = static_cast<int>(env->minPeaks.size());
    for (int x = 0; x < w; ++x) {
        double normX = static_cast<double>(x) / static_cast<double>(w);
        double nextNormX = static_cast<double>(x + 1) / static_cast<double>(w);

        double clipTime0 = normX * duration;
        double clipTime1 = nextNormX * duration;

        double srcTime0 = rev ? (env->durationSeconds - (offsetSec + clipTime1 * speedRatio))
                              : (offsetSec + clipTime0 * speedRatio);
        double srcTime1 = rev ? (env->durationSeconds - (offsetSec + clipTime0 * speedRatio))
                              : (offsetSec + clipTime1 * speedRatio);

        int bin0 = static_cast<int>(std::floor(srcTime0 * env->binsPerSecond));
        int bin1 = static_cast<int>(std::ceil(srcTime1 * env->binsPerSecond));
        if (bin1 <= bin0) bin1 = bin0 + 1;

        if (bin0 >= totalBins || bin1 <= 0) continue;
        bin0 = std::clamp(bin0, 0, totalBins - 1);
        bin1 = std::clamp(bin1, bin0 + 1, totalBins);

        float minVal = 0.0f;
        float maxVal = 0.0f;
        for (int b = bin0; b < bin1; ++b) {
            minVal = std::min(minVal, env->minPeaks[static_cast<size_t>(b)]);
            maxVal = std::max(maxVal, env->maxPeaks[static_cast<size_t>(b)]);
        }

        // Apply clip gain + fade envelope visually
        float fadeGain = 1.0f;
        if (fadeInSec > 0.001 && clipTime0 < fadeInSec)
            fadeGain *= static_cast<float>(clipTime0 / fadeInSec);
        if (fadeOutSec > 0.001 && clipTime0 > (duration - fadeOutSec))
            fadeGain *= static_cast<float>(std::max(0.0, (duration - clipTime0) / fadeOutSec));

        float scale = gainLinear * fadeGain;
        minVal = std::clamp(minVal * scale, -1.0f, 1.0f);
        maxVal = std::clamp(maxVal * scale, -1.0f, 1.0f);

        int y1 = midY - static_cast<int>(maxVal * (midY - 2));
        int y2 = midY - static_cast<int>(minVal * (midY - 2));
        if (y1 == y2) y2 = y1 + 1;

        painter->drawLine(x, y1, x, y2);
    }

    // Draw fade-in / fade-out shading curves
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(QColor(255, 255, 255, 200), 1.5));
    painter->setBrush(QColor(0, 0, 0, 85));

    if (fadeInSec > 0.01) {
        double fadeW = std::min(static_cast<double>(w), (fadeInSec / duration) * w);
        QPolygonF poly;
        poly << QPointF(0, 0) << QPointF(fadeW, 0) << QPointF(0, h);
        painter->drawPolygon(poly);
    }
    if (fadeOutSec > 0.01) {
        double fadeW = std::min(static_cast<double>(w), (fadeOutSec / duration) * w);
        QPolygonF poly;
        poly << QPointF(w - fadeW, 0) << QPointF(w, 0) << QPointF(w, h);
        painter->drawPolygon(poly);
    }
}
