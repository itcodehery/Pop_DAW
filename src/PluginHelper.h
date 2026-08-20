#pragma once
#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>

class PluginHelper
{
public:
    static void showPluginMenu(tracktion::engine::Engine& engine, 
                               juce::Component* anchor, 
                               std::function<void(const juce::PluginDescription&)> onPluginSelected)
    {
        auto& pm = engine.getPluginManager();
        auto& knownList = pm.knownPluginList;
        
        juce::PopupMenu menu;
        
        // Let's create a tree of plugins
        if (auto tree = knownList.createTree(knownList.getTypes(), juce::KnownPluginList::sortByManufacturer))
        {
            for (auto* subTree : tree->subFolders)
            {
                juce::PopupMenu subMenu;
                for (const auto& p : subTree->plugins)
                {
                    subMenu.addItem(p.name, [onPluginSelected, desc = p]() {
                        onPluginSelected(desc);
                    });
                }
                menu.addSubMenu(subTree->folder, subMenu);
            }
            
            for (const auto& p : tree->plugins)
            {
                menu.addItem(p.name, [onPluginSelected, desc = p]() {
                    onPluginSelected(desc);
                });
            }
        }
        
        if (menu.getNumItems() == 0)
        {
            menu.addItem("No plugins found (Scan first)", false, false, nullptr);
        }
        
        menu.addSeparator();
        menu.addItem("Scan for Plugins...", [anchor, &engine]() {
            scanPluginsAsync(engine);
        });

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(anchor));
    }

private:
    static void scanPluginsAsync(tracktion::engine::Engine& engine)
    {
        // Simple async scan using JUCE
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, 
                                               "Scanning Plugins", 
                                               "Scanning for VST3 and LV2 plugins in standard directories... Check terminal for progress. This may take a while.");
                                               
        // In a real app, this should be on a background thread with a progress bar.
        // For simplicity, we just trigger a background thread here.
        juce::Thread::launch([&engine]() {
            auto& pm = engine.getPluginManager();
            auto& formatManager = pm.pluginFormatManager;
            auto& knownList = pm.knownPluginList;
            
            for (int i = 0; i < formatManager.getNumFormats(); ++i)
            {
                if (auto* format = formatManager.getFormat(i))
                {
                    juce::PluginDirectoryScanner scanner(knownList, *format, format->getDefaultLocationsToSearch(), true, juce::File(), true);
                    
                    juce::String name;
                    while (scanner.scanNextFile(true, name))
                    {
                        juce::Logger::writeToLog("Scanned: " + name);
                    }
                }
            }
            
            juce::Logger::writeToLog("Plugin scan complete! Found " + juce::String(knownList.getNumTypes()) + " plugins.");
            
            // Save to XML
            #if JUCE_64BIT
             auto settingId = tracktion::engine::SettingID::knownPluginList64;
            #else
             auto settingId = tracktion::engine::SettingID::knownPluginList;
            #endif
            
            if (auto xml = knownList.createXml())
            {
                engine.getPropertyStorage().setXmlProperty(settingId, *xml);
            }
        });
    }
};
