/*
  ==============================================================================

    Pop DAW - A modern Digital Audio Workstation for Linux
    Copyright (C) 2024 Pop DAW Contributors

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"

//==============================================================================
class PopDAWLogger : public juce::Logger
{
public:
    void logMessage(const juce::String& message) override
    {
        std::cout << message.toRawUTF8() << std::endl;
    }
};

//==============================================================================
class PopDAWApplication : public juce::JUCEApplication
{
public:
    PopDAWApplication() = default;

    const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override { return false; }

    //==============================================================================
    void initialise(const juce::String& /*commandLine*/) override
    {
        logger = std::make_unique<PopDAWLogger>();
        juce::Logger::setCurrentLogger(logger.get());
        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        mainWindow.reset();
        juce::Logger::setCurrentLogger(nullptr);
        logger.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String& /*commandLine*/) override {}

private:
    //==============================================================================
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(const juce::String& name)
            : DocumentWindow(name,
                             juce::Desktop::getInstance().getDefaultLookAndFeel()
                                 .findColour(ResizableWindow::backgroundColourId),
                             DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);

            #if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
            #else
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
            #endif

            setVisible(true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

    std::unique_ptr<PopDAWLogger> logger;
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION(PopDAWApplication)
