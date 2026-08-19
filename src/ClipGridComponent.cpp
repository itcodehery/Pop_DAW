#include "ClipGridComponent.h"

//==============================================================================
ClipBlockComponent::ClipBlockComponent(juce::String name, juce::String keyBind, juce::Colour trackColor)
    : clipName(name), keyBinding(keyBind), baseColor(trackColor)
{
}

void ClipBlockComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Fill block background
    g.setColour(isPlaying ? baseColor.brighter(0.2f) : baseColor);
    g.fillRoundedRectangle(bounds, 6.0f);
    
    // Draw block border
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.drawRoundedRectangle(bounds, 6.0f, 2.0f);
    
    // Top bar for name and key
    auto topBar = bounds.removeFromTop(20.0f);
    g.setColour(juce::Colours::black.withAlpha(0.2f));
    g.fillPath([](juce::Rectangle<float> r) {
        juce::Path p;
        p.addRoundedRectangle(r.getX(), r.getY(), r.getWidth(), r.getHeight(), 6.0f, 6.0f, true, true, false, false);
        return p;
    }(topBar));
    
    g.setColour(juce::Colour(0xff222222));
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText(clipName.toUpperCase(), topBar.reduced(6.0f, 0).toNearestInt(), juce::Justification::centredLeft);
    g.drawText("KEY : " + keyBinding.toUpperCase(), topBar.reduced(6.0f, 0).toNearestInt(), juce::Justification::centredRight);
    
    // Draw dummy waveform
    g.setColour(juce::Colour(0xff111111));
    auto waveArea = bounds.reduced(10.0f, 15.0f);
    float midY = waveArea.getCentreY();
    
    juce::Random rand((int)clipName.hashCode());
    for (float x = waveArea.getX(); x < waveArea.getRight(); x += 4.0f)
    {
        float amplitude = rand.nextFloat() * waveArea.getHeight() * 0.4f + 2.0f;
        g.fillRoundedRectangle(x, midY - amplitude, 2.5f, amplitude * 2.0f, 1.0f);
    }
}

void ClipBlockComponent::mouseDown(const juce::MouseEvent&)
{
    isPlaying = !isPlaying;
    repaint();
}

//==============================================================================
ClipGridColumn::ClipGridColumn(juce::Colour trackColor) : color(trackColor)
{
}

void ClipGridColumn::addClip(juce::String name, juce::String keyBind)
{
    clips.push_back(std::make_unique<ClipBlockComponent>(name, keyBind, color));
    addAndMakeVisible(clips.back().get());
    resized();
}

void ClipGridColumn::paint(juce::Graphics& g)
{
    // Draw vertical separator line on the right side
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRect(getWidth() - 1, 0, 1, getHeight());
}

void ClipGridColumn::resized()
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromRight(1); // Keep clear of the separator
    
    for (auto& clip : clips)
    {
        clip->setBounds(area.removeFromTop(100));
        area.removeFromTop(10); // Vertical spacing between clips
    }
}

//==============================================================================
ClipGridComponent::ClipGridComponent()
{
}

ClipGridComponent::~ClipGridComponent()
{
}

void ClipGridComponent::addColumn(juce::Colour trackColor)
{
    columns.push_back(std::make_unique<ClipGridColumn>(trackColor));
    addAndMakeVisible(columns.back().get());
    resized();
}

ClipGridColumn* ClipGridComponent::getColumn(int index)
{
    if (index >= 0 && static_cast<size_t>(index) < columns.size())
        return columns[static_cast<size_t>(index)].get();
    return nullptr;
}

void ClipGridComponent::paint(juce::Graphics&)
{
    // Background is inherited from MainComponent
}

void ClipGridComponent::resized()
{
    auto area = getLocalBounds();
    int colWidth = 170; // Must match the mixer channel width
    int spacing = 10;
    
    for (auto& col : columns)
    {
        col->setBounds(area.removeFromLeft(colWidth + spacing));
    }
}
