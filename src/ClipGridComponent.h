#pragma once
#include <JuceHeader.h>

class ClipBlockComponent : public juce::Component
{
public:
    ClipBlockComponent(juce::String name, juce::String keyBind, juce::Colour trackColor);
    
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    juce::String clipName;
    juce::String keyBinding;
    juce::Colour baseColor;
    bool isPlaying = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipBlockComponent)
};

class ClipGridColumn : public juce::Component
{
public:
    ClipGridColumn(juce::Colour trackColor);
    
    void addClip(juce::String name, juce::String keyBind);
    
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Colour color;
    std::vector<std::unique_ptr<ClipBlockComponent>> clips;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipGridColumn)
};

class ClipGridComponent : public juce::Component
{
public:
    ClipGridComponent();
    ~ClipGridComponent() override;

    void addColumn(juce::Colour trackColor);
    ClipGridColumn* getColumn(int index);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    std::vector<std::unique_ptr<ClipGridColumn>> columns;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipGridComponent)
};
