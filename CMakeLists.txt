cmake_minimum_required(VERSION 3.15...3.28)

project(PopDAW
    VERSION 0.1.0
    DESCRIPTION "Pop DAW - A modern Digital Audio Workstation for Linux"
    HOMEPAGE_URL "https://github.com/hery/Pop_DAW"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(POPDAW_BUILD_TESTS "Build tests" OFF)
option(POPDAW_USE_ASAN "Enable AddressSanitizer in Debug builds" OFF)
option(POPDAW_USE_TSAN "Enable ThreadSanitizer in Debug builds" OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(POPDAW_USE_ASAN)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    elseif(POPDAW_USE_TSAN)
        add_compile_options(-fsanitize=thread)
        add_link_options(-fsanitize=thread)
    endif()
endif()

# Add JUCE (bundled with tracktion_engine)
add_subdirectory(libs/tracktion_engine/modules/juce)

# Add Tracktion Engine modules
add_subdirectory(libs/tracktion_engine/modules)

# Adds all the module sources so they appear correctly in the IDE
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

# Create the main GUI application
juce_add_gui_app(PopDAW
    BUNDLE_ID "com.popdaw.app"
    PRODUCT_NAME "Pop DAW"
    COMPANY_NAME "Pop DAW"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Pop DAW needs microphone access for audio recording"
    NEEDS_CURL FALSE
    NEEDS_WEB_BROWSER FALSE
)

juce_generate_juce_header(PopDAW)

# Source files
target_sources(PopDAW PRIVATE
    src/Main.cpp
    src/MainComponent.cpp
    src/MainComponent.h
    src/PopLookAndFeel.cpp
    src/PopLookAndFeel.h
    src/MixerChannelComponent.cpp
    src/MixerChannelComponent.h
)

# Compile definitions
target_compile_definitions(PopDAW PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:PopDAW,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:PopDAW,VERSION>"
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_LV2=1
    JUCE_MODAL_LOOPS_PERMITTED=1
)

# Binary Data
juce_add_binary_data(PopDAW_Assets
    HEADER_NAME "BinaryData.h"
    NAMESPACE "BinaryData"
    SOURCES
        assets/fonts/BBHBartle-Regular.ttf
        assets/fonts/DMSans.ttf
)

# Link against JUCE modules and Tracktion Engine
target_link_libraries(PopDAW PRIVATE
    PopDAW_Assets
    tracktion::tracktion_engine
    tracktion::tracktion_graph
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_osc
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
