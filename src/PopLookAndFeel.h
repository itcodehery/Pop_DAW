#pragma once
#include <JuceHeader.h>

class PopLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PopLookAndFeel();
    
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle, juce::Slider&) override;
                          
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
                              
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, float sliderPos,
                          const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override;
                              
    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override;

private:
    juce::Typeface::Ptr uiTypeface;
};
