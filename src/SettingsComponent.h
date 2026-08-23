#pragma once
#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>

//==============================================================================
// Audio Settings Tab
class AudioSettingsPanel : public juce::Component
{
public:
    AudioSettingsPanel(tracktion::engine::Engine& engine);
    ~AudioSettingsPanel() override;
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioDeviceSelector;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioSettingsPanel)
};

//==============================================================================
// MIDI Settings Tab
class MidiSettingsPanel : public juce::Component
{
public:
    MidiSettingsPanel(tracktion::engine::Engine& engine);
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    tracktion::engine::Engine& engine;
    juce::Label titleLabel;
    juce::ToggleButton midiDeviceButtons[16]; // Up to 16 MIDI devices
    int numMidiDevices = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiSettingsPanel)
};

//==============================================================================
// Plugin Directories Tab
class PluginSettingsPanel : public juce::Component
{
public:
    PluginSettingsPanel(tracktion::engine::Engine& engine);
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    tracktion::engine::Engine& engine;
    juce::Label titleLabel;
    std::unique_ptr<juce::PluginListComponent> pluginListComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginSettingsPanel)
};

//==============================================================================
// Main Settings Component (replaces the DAW view)
class SettingsComponent : public juce::Component
{
public:
    SettingsComponent(tracktion::engine::Engine& engine);
    ~SettingsComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void()> onClose; // Callback to return to DAW view

private:
    tracktion::engine::Engine& engine;

    juce::TextButton closeButton { "← Back to DAW" };
    juce::TextButton audioTabButton { "Audio" };
    juce::TextButton midiTabButton { "MIDI" };
    juce::TextButton pluginsTabButton { "Plugins" };

    AudioSettingsPanel audioPanel;
    MidiSettingsPanel midiPanel;
    PluginSettingsPanel pluginPanel;

    enum class Tab { Audio, Midi, Plugins };
    Tab currentTab = Tab::Audio;
    void showTab(Tab tab);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
};
