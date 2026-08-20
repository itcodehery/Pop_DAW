#pragma once
#include <JuceHeader.h>

class ActiveControllerComponent : public juce::Component
{
public:
    ActiveControllerComponent();
    ~ActiveControllerComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::TextButton openPluginButton { "Open Plugin" };
    
    juce::Slider knob1 { juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::Slider knob2 { juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::Slider knob3 { juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::Slider knob4 { juce::Slider::Rotary, juce::Slider::NoTextBox };
    
    juce::Label label1 { {}, "ATTACK" };
    juce::Label label2 { {}, "RELEASE" };
    juce::Label label3 { {}, "DELAY" };
    juce::Label label4 { {}, "SUSTAIN" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ActiveControllerComponent)
};

class ActiveEffectsComponent : public juce::Component
{
public:
    ActiveEffectsComponent();
    ~ActiveEffectsComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    std::vector<juce::String> dummyEffects = { "Filter Delay", "Limiter", "Resonator", "EQ Eight Band" };
    juce::TextButton addEffectButton { "+ ADD AN EFFECT" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ActiveEffectsComponent)
};

class SidebarComponent : public juce::Component
{
public:
    SidebarComponent();
    ~SidebarComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    ActiveControllerComponent activeController;
    ActiveEffectsComponent activeEffects;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SidebarComponent)
};
