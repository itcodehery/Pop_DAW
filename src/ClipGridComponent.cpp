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
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // Draw block border (solid thin black)
    g.setColour(juce::Colour(0xff111111));
    g.drawRoundedRectangle(bounds, 4.0f, 1.5f);
    
    // Top text
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    auto textArea = bounds.removeFromTop(20.0f).reduced(6.0f, 0).toNearestInt();
    g.drawText(clipName.toUpperCase(), textArea, juce::Justification::centredLeft);
    g.drawText("KEY : " + keyBinding.toUpperCase(), textArea, juce::Justification::centredRight);
    
    // Draw dummy waveform
    auto waveArea = bounds.reduced(10.0f, 10.0f);
    float midY = waveArea.getCentreY();
    
    juce::Random rand((int)clipName.hashCode());
    for (float x = waveArea.getX(); x < waveArea.getRight(); x += 4.0f)
    {
        float amplitude = rand.nextFloat() * waveArea.getHeight() * 0.4f + 2.0f;
        g.fillRoundedRectangle(x, midY - amplitude, 2.5f, amplitude * 2.0f, 1.5f);
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

void ClipGridColumn::paint(juce::Graphics&)
{
}

void ClipGridColumn::resized()
{
    auto area = getLocalBounds().reduced(2, 10);
    
    for (auto& clip : clips)
    {
        clip->setBounds(area.removeFromTop(90));
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

void ClipGridComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff111111));
    
    // Row markers
    int y = 10;
    for (int i=0; i<10; ++i) {
        g.fillRect(10, y + 45, 10, 1);
        y += 100; // 90 block height + 10 spacing
    }
}

void ClipGridComponent::resized()
{
    auto area = getLocalBounds();
    area.removeFromLeft(30); // Leave space for gutter
    area.reduce(20, 10);     // Padding
    
    int colWidth = 190;
    int spacing = 10;
    
    for (auto& col : columns)
    {
        col->setBounds(area.removeFromLeft(colWidth));
        area.removeFromLeft(spacing);
    }
}
