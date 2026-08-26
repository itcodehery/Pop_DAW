#include "WaveformItem.h"
#include "EngineController.h"

WaveformItem::WaveformItem(QQuickItem* parent) : QQuickPaintedItem(parent) {}

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
    
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(QPen(QColor("#111111"), 1.0));
    
    auto duration = waveClip->getPosition().time.getLength().inSeconds();
    if (duration <= 0) return;
    
    int w = boundingRect().width();
    int h = boundingRect().height();
    if (w <= 0 || h <= 0) return;
    int midY = h / 2;
    
    auto audioFile = waveClip->getAudioFile();
    if (!audioFile.isValid()) return;

    // Use Tracktion's format manager to get a reader
    auto& fm = m_controller->getEdit()->engine.getAudioFileFormatManager().readFormatManager;
    std::unique_ptr<juce::AudioFormatReader> reader(fm.createReaderFor(audioFile.getFile()));
    if (!reader) {
        painter->drawLine(0, midY, w, midY);
        return;
    }

    float startTime = waveClip->getPosition().offset.inSeconds();
    float endTime = startTime + duration;
    
    juce::int64 startSample = (juce::int64)(startTime * reader->sampleRate);
    juce::int64 endSample = (juce::int64)(endTime * reader->sampleRate);
    juce::int64 numSamples = endSample - startSample;
    if (numSamples <= 0) return;
    
    juce::int64 samplesPerPixel = numSamples / w;
    if (samplesPerPixel < 1) samplesPerPixel = 1;
    
    // Read the whole block at once to avoid blocking UI thread with thousands of small disk reads
    // Cap memory usage to avoid massive allocations (e.g., if zoomed out a lot)
    juce::int64 maxSamplesToRead = juce::jmin(numSamples, (juce::int64)4410000); // Max 100 seconds at a time
    juce::AudioBuffer<float> buffer(1, (int)maxSamplesToRead);
    reader->read(&buffer, 0, (int)maxSamplesToRead, startSample, true, false);
    
    auto* data = buffer.getReadPointer(0);
    
    for (int x = 0; x < w; ++x) {
        juce::int64 posInBuf = x * samplesPerPixel;
        if (posInBuf >= maxSamplesToRead) break;
        
        float minVal = 0.0f;
        float maxVal = 0.0f;
        
        juce::int64 samplesToCheck = juce::jmin(samplesPerPixel, maxSamplesToRead - posInBuf);
        for (int i = 0; i < samplesToCheck; ++i) {
            float v = data[posInBuf + i];
            if (v < minVal) minVal = v;
            if (v > maxVal) maxVal = v;
        }
        
        int y1 = midY - (int)(maxVal * midY);
        int y2 = midY - (int)(minVal * midY);
        if (y1 == y2) y2 = y1 + 1;
        
        painter->drawLine(x, y1, x, y2);
    }
}
