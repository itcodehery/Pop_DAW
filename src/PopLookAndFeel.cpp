#include "PopLookAndFeel.h"
#include <BinaryData.h>

PopLookAndFeel::PopLookAndFeel()
{
    uiTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::DMSans_ttf, BinaryData::DMSans_ttfSize);
}

juce::Typeface::Ptr PopLookAndFeel::getTypefaceForFont(const juce::Font& font)
{
    if (uiTypeface != nullptr && (font.getTypefaceName() == juce::Font::getDefaultSansSerifFontName() || font.getTypefaceName() == "<sans-serif>"))
        return uiTypeface;
        
    return juce::LookAndFeel_V4::getTypefaceForFont(font);
}

void PopLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                      float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                      const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::LinearVertical)
    {
        auto trackWidth = 2.0f;
        auto trackX = x + width * 0.5f - trackWidth * 0.5f;
        
        // Draw track
        g.setColour(juce::Colours::white);
        g.fillRect(trackX, (float)y, trackWidth, (float)height);
        
        // Draw thumb (square)
        auto thumbSize = 12.0f;
        auto thumbX = x + width * 0.5f - thumbSize * 0.5f;
        
        // Thumb color
        g.setColour(slider.findColour(juce::Slider::thumbColourId, true));
        g.fillRect(thumbX, sliderPos - thumbSize * 0.5f, thumbSize, thumbSize);
    }
    else
    {
        // Fallback
        g.setColour(juce::Colours::red);
        g.fillRect(x, y, width, height);
    }
}

void PopLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    
    auto baseColour = backgroundColour;
    if (shouldDrawButtonAsDown || button.getToggleState())
        baseColour = baseColour.brighter(0.2f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter(0.1f);
        
    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, 4.0f);
}

void PopLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                      const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Track Background
    juce::Path backgroundArc;
    backgroundArc.addCentredArc (centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colours::white.withAlpha(0.2f));
    g.strokePath (backgroundArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));

    // Value Arc
    juce::Path valueArc;
    valueArc.addCentredArc (centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (slider.findColour(juce::Slider::rotarySliderFillColourId));
    g.strokePath (valueArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
    
    // Pointer line
    juce::Path pointer;
    auto pointerLength = radius * 0.4f;
    pointer.startNewSubPath(centreX, centreY);
    pointer.lineTo(centreX + pointerLength * std::sin (angle),
                   centreY - pointerLength * std::cos (angle));
    g.strokePath (pointer, juce::PathStrokeType (2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
}
