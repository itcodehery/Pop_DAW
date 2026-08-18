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
