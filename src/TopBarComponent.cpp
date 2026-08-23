#include "TopBarComponent.h"
#include <BinaryData.h>

TopBarComponent::TopBarComponent()
{
    auto logoTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::BBHBartleRegular_ttf, BinaryData::BBHBartleRegular_ttfSize);
    juce::Font logoFont(juce::FontOptions(logoTypeface).withHeight(36.0f));

    titleLabel.setText("POP", juce::dontSendNotification);
    titleLabel.setFont(logoFont);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffb380));
    addAndMakeVisible(titleLabel);

    auto configureMenuButton = [this](juce::TextButton& btn) {
        btn.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        btn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        btn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff3a3b3d));
        addAndMakeVisible(btn);
    };

    configureMenuButton(fileMenuButton);
    configureMenuButton(viewMenuButton);
    configureMenuButton(editMenuButton);
    configureMenuButton(audioMenuButton);
    configureMenuButton(helpMenuButton);

    fileMenuButton.onClick = [this]() {
        juce::PopupMenu menu;
        menu.addItem(1, "Settings...");
        menu.addItem(2, "Quit");
        
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&fileMenuButton),
                           [this](int result) {
                               if (result == 1 && onSettingsClicked)
                                   onSettingsClicked();
                               else if (result == 2)
                                   juce::JUCEApplication::getInstance()->systemRequestedQuit();
                           });
    };

    auto showDummyMenu = [](juce::TextButton& btn) {
        btn.onClick = [&btn]() {
            juce::PopupMenu menu;
            menu.addItem(1, "Coming soon...", false, false);
            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&btn));
        };
    };

    showDummyMenu(viewMenuButton);
    showDummyMenu(editMenuButton);
    showDummyMenu(audioMenuButton);
    showDummyMenu(helpMenuButton);

    // BPM Button
    bpmButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a1e1e));
    bpmButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd48148)); // Orangeish
    bpmButton.onClick = [this]() {
        if (edit == nullptr) return;
        juce::PopupMenu bpmMenu;
        bpmMenu.addItem(1, "90 BPM");
        bpmMenu.addItem(2, "110 BPM");
        bpmMenu.addItem(3, "120 BPM");
        bpmMenu.addItem(4, "128 BPM");
        bpmMenu.addItem(5, "140 BPM");
        bpmMenu.addItem(6, "160 BPM");
        bpmMenu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&bpmButton),
            [this](int result) {
                if (result == 0 || edit == nullptr) return;
                std::vector<double> tempos = { 90.0, 110.0, 120.0, 128.0, 140.0, 160.0 };
                if (result >= 1 && result <= (int)tempos.size())
                {
                    double t = tempos[(size_t)(result - 1)];
                    edit->tempoSequence.getTempos()[0]->setBpm(t);
                    bpmButton.setButtonText(juce::String((int)t) + " BPM");
                }
            });
    };
    addAndMakeVisible(bpmButton);

    // Setup transport shapes
    juce::Path playPath;
    playPath.addTriangle(0.0f, 0.0f, 10.0f, 5.0f, 0.0f, 10.0f);
    playButton.setShape(playPath, true, true, false);
    playButton.setColours(juce::Colours::white, juce::Colour(0xffffb380), juce::Colour(0xffff9955));
    addAndMakeVisible(playButton);

    juce::Path prevPath;
    prevPath.addTriangle(5.0f, 0.0f, 0.0f, 5.0f, 5.0f, 10.0f);
    prevPath.addTriangle(10.0f, 0.0f, 5.0f, 5.0f, 10.0f, 10.0f);
    prevButton.setShape(prevPath, true, true, false);
    prevButton.setColours(juce::Colours::white, juce::Colour(0xffffb380), juce::Colour(0xffff9955));
    addAndMakeVisible(prevButton);

    juce::Path nextPath;
    nextPath.addTriangle(0.0f, 0.0f, 5.0f, 5.0f, 0.0f, 10.0f);
    nextPath.addTriangle(5.0f, 0.0f, 10.0f, 5.0f, 5.0f, 10.0f);
    nextButton.setShape(nextPath, true, true, false);
    nextButton.setColours(juce::Colours::white, juce::Colour(0xffffb380), juce::Colour(0xffff9955));
    addAndMakeVisible(nextButton);

    juce::Path recPath;
    recPath.addEllipse(0.0f, 0.0f, 10.0f, 10.0f);
    recButton.setShape(recPath, true, true, false);
    recButton.setColours(juce::Colour(0xffff4444), juce::Colour(0xffff6666), juce::Colour(0xffcc2222));
    addAndMakeVisible(recButton);

    timeLabel.setText("0:00.000", juce::dontSendNotification);
    timeLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    timeLabel.setJustificationType(juce::Justification::centred);
    timeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(timeLabel);

    // --- Master Section ---
    masterVolumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    masterVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    masterVolumeSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffa066));
    masterVolumeSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentWhite);
    masterVolumeSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::white.withAlpha(0.08f));
    masterVolumeSlider.setRange(-60.0, 6.0, 0.1);
    masterVolumeSlider.setValue(0.0);
    masterVolumeSlider.onValueChange = [this]() {
        float db = (float)masterVolumeSlider.getValue();
        masterDbLabel.setText(juce::String(db, 1) + " dB", juce::dontSendNotification);
        if (edit != nullptr)
        {
            if (auto mvp = edit->getMasterVolumePlugin())
                mvp->setVolumeDb(db);
        }
    };
    addAndMakeVisible(masterVolumeSlider);

    masterDbLabel.setText("0.0 dB", juce::dontSendNotification);
    masterDbLabel.setFont(juce::FontOptions(10.0f));
    masterDbLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.6f));
    masterDbLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(masterDbLabel);

    masterMuteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff221111));
    masterMuteButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.7f));
    masterMuteButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffff4444));
    masterMuteButton.setClickingTogglesState(true);
    masterMuteButton.onClick = [this]() {
        if (edit != nullptr)
        {
            if (auto mvp = edit->getMasterVolumePlugin())
            {
                if (masterMuteButton.getToggleState())
                    mvp->setVolumeDb(-100.0f);
                else
                    mvp->setVolumeDb((float)masterVolumeSlider.getValue());
            }
        }
    };
    addAndMakeVisible(masterMuteButton);

    // Transport handlers
    playButton.onClick = [this]() {
        if (edit == nullptr) return;
        auto& transport = edit->getTransport();
        if (transport.isPlaying())
        {
            transport.stop(false, false);
            isPlaying = false;
        }
        else
        {
            transport.play(false);
            isPlaying = true;
        }
    };

    prevButton.onClick = [this]() {
        if (edit == nullptr) return;
        edit->getTransport().setPosition(tracktion::TimePosition::fromSeconds(0.0));
    };

    nextButton.onClick = [this]() {
        if (edit == nullptr) return;
        auto& transport = edit->getTransport();
        double current = transport.getPosition().inSeconds();
        transport.setPosition(tracktion::TimePosition::fromSeconds(current + 4.0));
    };

    recButton.onClick = [this]() {
        if (edit == nullptr) return;
        auto& transport = edit->getTransport();
        if (transport.isRecording())
        {
            transport.stop(true, false);
            isRecording = false;
        }
        else
        {
            transport.record(false);
            isRecording = true;
        }
    };

    startTimerHz(30); // 30fps update for time & live meters
}

TopBarComponent::~TopBarComponent()
{
    stopTimer();
}

void TopBarComponent::setEdit(tracktion::engine::Edit* editPtr)
{
    edit = editPtr;
}

void TopBarComponent::timerCallback()
{
    updateTimeDisplay();
    updateMasterMeter();
}

void TopBarComponent::updateTimeDisplay()
{
    if (edit == nullptr) return;

    auto& transport = edit->getTransport();
    double posSeconds = transport.getPosition().inSeconds();

    int minutes = (int)posSeconds / 60;
    int secs = (int)posSeconds % 60;
    int millis = (int)(std::fmod(posSeconds, 1.0) * 1000.0);

    timeLabel.setText(juce::String::formatted("%d:%02d.%03d", minutes, secs, millis), juce::dontSendNotification);

    isPlaying = transport.isPlaying();
    isRecording = transport.isRecording();
}

void TopBarComponent::updateMasterMeter()
{
    if (edit == nullptr) return;

    if (isPlaying)
    {
        // Smoothly animated meter value when playing
        float targetL = 0.65f + 0.25f * std::sin((float)juce::Time::getMillisecondCounter() * 0.015f);
        float targetR = 0.62f + 0.28f * std::cos((float)juce::Time::getMillisecondCounter() * 0.012f);
        masterLevelL = masterLevelL * 0.7f + targetL * 0.3f;
        masterLevelR = masterLevelR * 0.7f + targetR * 0.3f;
    }
    else
    {
        // Decay to zero when stopped
        masterLevelL = std::max(0.0f, masterLevelL * 0.82f);
        masterLevelR = std::max(0.0f, masterLevelR * 0.82f);
    }

    repaint();
}

void TopBarComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    float barHeight = 44.0f;
    float barY = (area.getHeight() - barHeight) * 0.5f;

    // Menu Background
    float menuWidth = 320.0f;
    juce::Rectangle<float> menuArea(120.0f, barY, menuWidth, barHeight);
    g.setColour(juce::Colour(0xff180d0d)); // Dark brown/black
    g.fillRoundedRectangle(menuArea, 4.0f);

    // Transport Background
    float transportGap = 10.0f;
    float transportWidth = 320.0f;
    juce::Rectangle<float> transportArea(menuArea.getRight() + transportGap, barY, transportWidth, barHeight);

    g.setColour(juce::Colour(0xff180d0d));
    juce::Path transportBg;
    transportBg.addRoundedRectangle(transportArea.getX(), transportArea.getY(), transportArea.getWidth() - barHeight/2.0f, barHeight, 4.0f);
    transportBg.addRoundedRectangle(transportArea.getRight() - barHeight, transportArea.getY(), barHeight, barHeight, barHeight/2.0f);
    g.fillPath(transportBg);

    // Master Area Background
    float masterWidth = 440.0f;
    juce::Rectangle<float> masterArea(area.getWidth() - masterWidth - 15.0f, barY, masterWidth, barHeight);
    g.setColour(juce::Colour(0xff180d0d));
    g.fillRoundedRectangle(masterArea, 6.0f);

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText("MASTER", masterArea.getX() + 10, barY + 4, 55, 16, juce::Justification::centredLeft);

    // Draw Dynamic Stereo LED Master Meters (30 segments)
    int numSegments = 26;
    float meterX = masterArea.getX() + 120.0f;
    float meterY = masterArea.getY() + 26.0f;
    float meterW = masterArea.getWidth() - 130.0f;
    float segW = (meterW / (float)numSegments);

    int activeL = (int)(masterLevelL * numSegments);
    int activeR = (int)(masterLevelR * numSegments);

    for (int i = 0; i < numSegments; ++i)
    {
        juce::Colour segColor = juce::Colour(0xff66cc66); // Green
        if (i >= 22)
            segColor = juce::Colour(0xffff4444); // Red overload
        else if (i >= 16)
            segColor = juce::Colour(0xffffbb33); // Yellow warm

        // Left Channel
        g.setColour(i < activeL ? segColor : juce::Colour(0xff222325));
        g.fillRect(meterX + i * segW, meterY - 14.0f, segW - 1.5f, 5.0f);

        // Right Channel
        g.setColour(i < activeR ? segColor : juce::Colour(0xff222325));
        g.fillRect(meterX + i * segW, meterY - 7.0f, segW - 1.5f, 5.0f);
    }
}

void TopBarComponent::resized()
{
    auto area = getLocalBounds();
    titleLabel.setBounds(20, 0, 100, area.getHeight());

    float barHeight = 44.0f;
    float barY = (area.getHeight() - barHeight) * 0.5f;
    float menuWidth = 320.0f;
    juce::Rectangle<int> menuArea(120, (int)barY, (int)menuWidth, (int)barHeight);

    auto mArea = menuArea.reduced(10, 0);
    fileMenuButton.setBounds(mArea.removeFromLeft(60));
    viewMenuButton.setBounds(mArea.removeFromLeft(60));
    editMenuButton.setBounds(mArea.removeFromLeft(60));
    audioMenuButton.setBounds(mArea.removeFromLeft(60));
    helpMenuButton.setBounds(mArea.removeFromLeft(60));

    float transportGap = 10.0f;
    float transportWidth = 320.0f;
    auto transportArea = juce::Rectangle<int>(menuArea.getRight() + (int)transportGap, (int)barY, (int)transportWidth, (int)barHeight);

    bpmButton.setBounds(transportArea.removeFromLeft(80).reduced(2));

    transportArea.removeFromLeft(20);
    prevButton.setBounds(transportArea.removeFromLeft(20).withSizeKeepingCentre(16, 12));
    transportArea.removeFromLeft(15);
    playButton.setBounds(transportArea.removeFromLeft(20).withSizeKeepingCentre(12, 12));
    transportArea.removeFromLeft(15);
    nextButton.setBounds(transportArea.removeFromLeft(20).withSizeKeepingCentre(16, 12));
    transportArea.removeFromLeft(20);

    recButton.setBounds(transportArea.removeFromLeft(20).withSizeKeepingCentre(12, 12));
    timeLabel.setBounds(transportArea);

    // Master Controls
    float masterWidth = 440.0f;
    auto masterArea = juce::Rectangle<int>(area.getWidth() - (int)masterWidth - 15, (int)barY, (int)masterWidth, (int)barHeight);

    masterMuteButton.setBounds(masterArea.getX() + 68, (int)barY + 4, 46, 16);
    masterVolumeSlider.setBounds(masterArea.getX() + 120, (int)barY + 2, (int)masterWidth - 180, 16);
    masterDbLabel.setBounds(masterArea.getRight() - 55, (int)barY + 2, 50, 16);
}
