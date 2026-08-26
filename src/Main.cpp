/*
  ==============================================================================

    Pop DAW - A modern Digital Audio Workstation for Linux
    Copyright (C) 2024 Pop DAW Contributors

  ==============================================================================
*/

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>

#include <QQmlContext>
#include <QQmlEngine>
#include <QFontDatabase>
#include <QFont>
#include "EngineController.h"
#include "TrackListModel.h"
#include "ClipListModel.h"

int main(int argc, char *argv[])
{
    qmlRegisterUncreatableType<ClipListModel>("PopDAW", 1, 0, "ClipListModel", "Used for roles only");

    // Initialize JUCE completely so Tracktion works
    juce::ScopedJuceInitialiser_GUI juceInit;

    QGuiApplication app(argc, argv);

    int id = QFontDatabase::addApplicationFont(":/fonts/DMSans.ttf");
    if (id != -1) {
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);
        QGuiApplication::setFont(QFont(family));
    }
    
    // Tracktion engine instance
    auto engine = std::make_unique<tracktion::engine::Engine>("Pop DAW");
    engine->getDeviceManager().initialise(0, 2);
    engine->getPluginManager().initialise();

    juce::File editFile = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("PopDAW_Session.tracktionedit");
    auto currentEdit = tracktion::engine::createEmptyEdit(*engine, editFile);
    currentEdit->getTransport().ensureContextAllocated();

    EngineController engineController(*engine, currentEdit.get());
    TrackListModel trackListModel(currentEdit.get());

    // Pump JUCE messages using Qt's event loop
    QTimer juceEventPump;
    QObject::connect(&juceEventPump, &QTimer::timeout, [] {
        juce::MessageManager::getInstance()->runDispatchLoopUntil(2);
    });
    juceEventPump.start(10); // Pump every 10ms

    QQmlApplicationEngine qmlEngine;
    qmlEngine.rootContext()->setContextProperty("engineController", &engineController);
    qmlEngine.rootContext()->setContextProperty("trackListModel", &trackListModel);

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    qmlEngine.load(url);

    return app.exec();
}
