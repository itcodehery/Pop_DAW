#include "ClipGridComponent.h"

//==============================================================================
// TimelineClipComponent
//==============================================================================
TimelineClipComponent::TimelineClipComponent(const juce::String& name, juce::Colour trackColor,
                                             double startTime, double lengthSeconds,
                                             juce::AudioFormatManager& fmtManager,
                                             juce::AudioThumbnailCache& thumbCache,
                                             const juce::File& audioFile,
                                             bool isMidi)
    : clipName(name), baseColor(trackColor), clipStartTime(startTime), clipLength(lengthSeconds),
      isMidiClip(isMidi), sourceAudioFile(audioFile)
{
    if (!isMidiClip)
    {
        thumbnail = std::make_unique<juce::AudioThumbnail>(512, fmtManager, thumbCache);
        if (sourceAudioFile.existsAsFile())
            thumbnail->setSource(new juce::FileInputSource(sourceAudioFile));
    }
}

void TimelineClipComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    // Fill block background
    g.setColour(baseColor.withAlpha(isDragging ? 0.9f : 0.75f));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Draw block border
    g.setColour(isDragging ? juce::Colours::white : baseColor.brighter(0.4f));
    g.drawRoundedRectangle(bounds, 4.0f, isDragging ? 1.5f : 1.0f);

    // Header badge area
    auto headerArea = bounds.removeFromTop(18.0f).reduced(4.0f, 2.0f);
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText(clipName, headerArea.withTrimmedRight(40.0f), juce::Justification::centredLeft, true);

    // Duration badge
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(juce::FontOptions(9.0f));
    g.drawText(juce::String(clipLength, 1) + "s", headerArea.withTrimmedLeft(headerArea.getWidth() - 36.0f), juce::Justification::centredRight);

    // Waveform / MIDI Area
    auto contentArea = bounds.reduced(4.0f, 2.0f);
    if (contentArea.getHeight() > 6.0f && contentArea.getWidth() > 10.0f)
    {
        if (isMidiClip)
        {
            // Draw stylized MIDI note blocks
            g.setColour(juce::Colours::white.withAlpha(0.85f));
            juce::Random rand((int)clipName.hashCode());
            float rowHeight = contentArea.getHeight() / 6.0f;
            for (float x = contentArea.getX() + 4.0f; x < contentArea.getRight() - 10.0f;)
            {
                int row = rand.nextInt(6);
                float noteLen = 6.0f + rand.nextFloat() * 18.0f;
                float noteY = contentArea.getY() + row * rowHeight + 1.0f;
                g.fillRoundedRectangle(x, noteY, noteLen, std::max(2.0f, rowHeight - 2.0f), 1.5f);
                x += noteLen + 4.0f + rand.nextFloat() * 8.0f;
            }
        }
        else if (thumbnail != nullptr && thumbnail->getTotalLength() > 0.0)
        {
            // Draw real audio waveform
            g.setColour(juce::Colours::white.withAlpha(0.85f));
            thumbnail->drawChannels(g, contentArea.toNearestInt(), 0.0, clipLength, 1.0f);
        }
        else
        {
            // Fallback stylized audio envelope
            float midY = contentArea.getCentreY();
            g.setColour(juce::Colours::white.withAlpha(0.7f));
            juce::Random rand((int)clipName.hashCode());
            for (float x = contentArea.getX(); x < contentArea.getRight(); x += 3.0f)
            {
                float amplitude = rand.nextFloat() * contentArea.getHeight() * 0.4f + 1.0f;
                g.fillRect(x, midY - amplitude, 2.0f, amplitude * 2.0f);
            }
        }
    }

    // Right edge resize handle indicator
    g.setColour(juce::Colours::white.withAlpha(0.4f));
    g.fillRect(bounds.getRight() - 3.0f, bounds.getY() + 6.0f, 2.0f, bounds.getHeight() - 12.0f);
}

void TimelineClipComponent::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        showContextMenu();
        return;
    }

    dragStartClipTime = clipStartTime;
    dragStartClipLength = clipLength;
    dragStartMouseX = e.getScreenX();

    // If mouse is within 8px of right edge, start resizing instead of moving
    if (e.x >= getWidth() - 8)
    {
        isResizing = true;
        isDragging = false;
    }
    else
    {
        isDragging = true;
        isResizing = false;
    }

    repaint();
}

void TimelineClipComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (auto* parentLane = dynamic_cast<TimelineTrackLane*>(getParentComponent()))
    {
        double pxPerSec = parentLane->getPxPerSec();
        if (pxPerSec <= 0.001) return;

        double deltaSeconds = (e.getScreenX() - dragStartMouseX) / pxPerSec;

        if (isResizing)
        {
            clipLength = std::max(0.2, dragStartClipLength + deltaSeconds);
        }
        else if (isDragging)
        {
            clipStartTime = std::max(0.0, dragStartClipTime + deltaSeconds);
        }

        parentLane->layoutClips(pxPerSec, parentLane->getScrollOffset());
        if (onClipChanged) onClipChanged(this);
    }
}

void TimelineClipComponent::mouseUp(const juce::MouseEvent&)
{
    isDragging = false;
    isResizing = false;
    repaint();
}

void TimelineClipComponent::showContextMenu()
{
    juce::PopupMenu menu;
    menu.addItem(1, "Delete Clip");
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
        [this](int result) {
            if (result == 1)
            {
                if (onDeleteClip) onDeleteClip(this);
            }
        });
}

//==============================================================================
// TimelineTrackLane
//==============================================================================
TimelineTrackLane::TimelineTrackLane(juce::Colour trackColor,
                                     juce::AudioFormatManager& fmtManager,
                                     juce::AudioThumbnailCache& tCache)
    : color(trackColor), formatManager(fmtManager), thumbCache(tCache)
{
}

void TimelineTrackLane::addAudioClip(const juce::String& name, double startTime, double lengthSeconds, const juce::File& file)
{
    auto clip = std::make_unique<TimelineClipComponent>(name, color, startTime, lengthSeconds, formatManager, thumbCache, file, false);
    clip->onDeleteClip = [this](TimelineClipComponent* c) { removeClip(c); };
    clips.push_back(std::move(clip));
    addAndMakeVisible(clips.back().get());
    resized();
}

void TimelineTrackLane::addMidiClip(const juce::String& name, double startTime, double lengthSeconds)
{
    auto clip = std::make_unique<TimelineClipComponent>(name, color, startTime, lengthSeconds, formatManager, thumbCache, juce::File(), true);
    clip->onDeleteClip = [this](TimelineClipComponent* c) { removeClip(c); };
    clips.push_back(std::move(clip));
    addAndMakeVisible(clips.back().get());
    resized();
}

void TimelineTrackLane::removeClip(TimelineClipComponent* clip)
{
    for (auto it = clips.begin(); it != clips.end(); ++it)
    {
        if (it->get() == clip)
        {
            clips.erase(it);
            resized();
            break;
        }
    }
}

void TimelineTrackLane::setTrackColor(juce::Colour c)
{
    color = c;
    for (auto& clip : clips)
        clip->setBaseColor(color);
    repaint();
}

void TimelineTrackLane::layoutClips(double pixelsPerSecond, double scrollOffsetSeconds)
{
    pxPerSec = pixelsPerSecond;
    scrollOffset = scrollOffsetSeconds;

    auto area = getLocalBounds();

    for (auto& clip : clips)
    {
        int xPos = (int)((clip->getStartTime() - scrollOffset) * pxPerSec);
        int width = std::max(20, (int)(clip->getLengthSeconds() * pxPerSec));
        clip->setBounds(xPos, 0, width, area.getHeight());
    }
}

void TimelineTrackLane::paint(juce::Graphics&)
{
}

void TimelineTrackLane::resized()
{
    layoutClips(pxPerSec, scrollOffset);
}

void TimelineTrackLane::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu() || e.getNumberOfClicks() >= 2)
    {
        double clickedTime = scrollOffset + (e.x / pxPerSec);
        if (clickedTime < 0.0) clickedTime = 0.0;

        juce::PopupMenu menu;
        menu.addItem(1, "Create MIDI Clip Here");
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
            [this, clickedTime](int result) {
                if (result == 1)
                {
                    addMidiClip("MIDI Clip", clickedTime, 4.0);
                    if (onAddMidiClipAtTime) onAddMidiClipAtTime(clickedTime);
                }
            });
    }
}

//==============================================================================
// TimelineComponent
//==============================================================================
TimelineComponent::TimelineComponent()
{
    formatManager.registerBasicFormats();
    startTimerHz(30); // 30fps update for playhead
}

TimelineComponent::~TimelineComponent()
{
    stopTimer();
}

void TimelineComponent::setEdit(tracktion::engine::Edit* editPtr)
{
    edit = editPtr;
}

int TimelineComponent::addTrackLane(juce::Colour trackColor)
{
    trackLanes.push_back(std::make_unique<TimelineTrackLane>(trackColor, formatManager, thumbnailCache));
    addAndMakeVisible(trackLanes.back().get());
    resized();
    return (int)trackLanes.size() - 1;
}

void TimelineComponent::removeTrackLane(int index)
{
    if (index >= 0 && static_cast<size_t>(index) < trackLanes.size())
    {
        trackLanes.erase(trackLanes.begin() + index);
        resized();
        repaint();
    }
}

TimelineTrackLane* TimelineComponent::getTrackLane(int index)
{
    if (index >= 0 && static_cast<size_t>(index) < trackLanes.size())
        return trackLanes[static_cast<size_t>(index)].get();
    return nullptr;
}

void TimelineComponent::addClipToLane(int laneIndex, const juce::String& name, double startTime, double lengthSeconds, const juce::File& file)
{
    if (auto* lane = getTrackLane(laneIndex))
    {
        lane->addAudioClip(name, startTime, lengthSeconds, file);
        layoutAllLanes();
    }
}

void TimelineComponent::addMidiClipToLane(int laneIndex, const juce::String& name, double startTime, double lengthSeconds)
{
    if (auto* lane = getTrackLane(laneIndex))
    {
        lane->addMidiClip(name, startTime, lengthSeconds);
        layoutAllLanes();
    }
}

void TimelineComponent::timerCallback()
{
    if (edit != nullptr)
    {
        auto& transport = edit->getTransport();
        double newPos = transport.getPosition().inSeconds();
        if (std::abs(newPos - playheadPosition) > 0.001)
        {
            playheadPosition = newPos;
            repaint();
        }
    }
}

void TimelineComponent::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (e.mods.isCtrlDown())
    {
        // Zoom in/out
        double zoomFactor = 1.0 + wheel.deltaY * 0.3;
        pixelsPerSecond = juce::jlimit(5.0, 500.0, pixelsPerSecond * zoomFactor);
        layoutAllLanes();
        repaint();
    }
    else
    {
        // Horizontal scroll
        double scrollDelta = -wheel.deltaY * 2.0; // seconds
        scrollOffsetSeconds = std::max(0.0, scrollOffsetSeconds + scrollDelta);
        layoutAllLanes();
        repaint();
    }
}

void TimelineComponent::mouseDown(const juce::MouseEvent& e)
{
    int rulerHeight = 28;
    if (e.y <= rulerHeight && edit != nullptr)
    {
        // Click on time ruler sets playhead position!
        double clickedTime = scrollOffsetSeconds + (e.x / pixelsPerSecond);
        if (clickedTime >= 0.0)
        {
            edit->getTransport().setPosition(tracktion::TimePosition::fromSeconds(clickedTime));
            playheadPosition = clickedTime;
            repaint();
        }
    }
}

void TimelineComponent::layoutAllLanes()
{
    auto area = getLocalBounds();
    int rulerHeight = 28;
    int laneHeight = 60;
    int spacing = 12;

    auto lanesArea = area.withTrimmedTop(rulerHeight + 10);

    for (auto& lane : trackLanes)
    {
        auto laneRect = lanesArea.removeFromTop(laneHeight);
        lane->setBounds(laneRect);
        lane->layoutClips(pixelsPerSecond, scrollOffsetSeconds);
        lanesArea.removeFromTop(spacing);
    }
}

void TimelineComponent::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff1e1f21));

    int rulerHeight = 28;
    int laneHeight = 60;
    int spacing = 12;

    // Draw vertical beat/second grid lines across the timeline
    g.setColour(juce::Colour(0xff2c2d30).withAlpha(0.6f));
    double gridIntervalSeconds = (pixelsPerSecond > 80.0) ? 1.0 : (pixelsPerSecond > 20.0 ? 2.0 : 4.0);

    double startSec = std::floor(scrollOffsetSeconds / gridIntervalSeconds) * gridIntervalSeconds;
    for (double t = startSec; ; t += gridIntervalSeconds)
    {
        int x = (int)((t - scrollOffsetSeconds) * pixelsPerSecond);
        if (x > getWidth()) break;
        if (x >= 0)
            g.drawVerticalLine(x, (float)rulerHeight, (float)getHeight());
    }

    // Draw alternating lane backgrounds
    float y = (float)(rulerHeight + 10);
    for (size_t i = 0; i < trackLanes.size(); ++i)
    {
        juce::Rectangle<float> laneBg(0.0f, y, (float)getWidth(), (float)laneHeight);
        g.setColour(juce::Colour(i % 2 == 0 ? 0xff252628 : 0xff212224));
        g.fillRect(laneBg);
        y += laneHeight + spacing;
    }

    // Draw Time Ruler
    drawTimeRuler(g);

    // Draw Playhead
    drawPlayhead(g);
}

void TimelineComponent::drawTimeRuler(juce::Graphics& g)
{
    int rulerHeight = 28;
    auto rulerBounds = getLocalBounds().removeFromTop(rulerHeight);

    g.setColour(juce::Colour(0xff161718));
    g.fillRect(rulerBounds);

    g.setColour(juce::Colour(0xff333438));
    g.drawHorizontalLine(rulerHeight - 1, 0.0f, (float)getWidth());

    // Major markings every 4 seconds or 2 seconds based on zoom
    double markInterval = (pixelsPerSecond > 80.0) ? 1.0 : (pixelsPerSecond > 30.0 ? 2.0 : 4.0);

    double startSec = std::floor(scrollOffsetSeconds / markInterval) * markInterval;
    g.setFont(juce::FontOptions(10.0f));

    for (double t = startSec; ; t += markInterval)
    {
        int x = (int)((t - scrollOffsetSeconds) * pixelsPerSecond);
        if (x > getWidth()) break;
        if (x < 0) continue;

        // Tick mark
        g.setColour(juce::Colour(0xff55565a));
        g.drawVerticalLine(x, (float)(rulerHeight - 8), (float)rulerHeight);

        // Time label: M:SS
        int minutes = (int)t / 60;
        int seconds = (int)t % 60;
        juce::String timeStr = juce::String::formatted("%d:%02d", minutes, seconds);

        g.setColour(juce::Colour(0xff888a90));
        g.drawText(timeStr, x + 4, 2, 40, rulerHeight - 4, juce::Justification::centredLeft);
    }
}

void TimelineComponent::drawPlayhead(juce::Graphics& g)
{
    int playheadX = (int)((playheadPosition - scrollOffsetSeconds) * pixelsPerSecond);
    if (playheadX >= 0 && playheadX <= getWidth())
    {
        // Playhead line
        g.setColour(juce::Colour(0xffffa066)); // Pastel orange
        g.fillRect((float)playheadX - 1.0f, 0.0f, 2.0f, (float)getHeight());

        // Playhead handle on ruler
        juce::Path handle;
        handle.addTriangle((float)playheadX - 6.0f, 0.0f,
                           (float)playheadX + 6.0f, 0.0f,
                           (float)playheadX, 12.0f);
        g.setColour(juce::Colour(0xffffa066));
        g.fillPath(handle);
    }
}
