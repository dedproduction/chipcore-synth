cmake_minimum_required(VERSION 3.22)

# Static MSVC runtime: the plugin works without installing the VC++ redistributable
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")

project(CHIPCORE VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- JUCE -------------------------------------------------------------------
# Pass -DJUCE_PATH=/path/to/JUCE to use a local copy, otherwise it is downloaded.
if(DEFINED JUCE_PATH)
    add_subdirectory(${JUCE_PATH} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.10
        GIT_SHALLOW    ON)
    FetchContent_MakeAvailable(JUCE)
endif()

set(CHIPCORE_FORMATS "VST3;Standalone" CACHE STRING "Plugin formats to build")

juce_add_plugin(CHIPCORE
    COMPANY_NAME                "dedproduction"
    COMPANY_WEBSITE             "https://github.com/dedproduction"
    PLUGIN_MANUFACTURER_CODE    Dedp
    PLUGIN_CODE                 Chpc
    FORMATS                     ${CHIPCORE_FORMATS}
    PRODUCT_NAME                "CHIPCORE"
    BUNDLE_ID                   com.dedproduction.chipcore
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             Instrument Synth)

juce_add_binary_data(ChipcoreData SOURCES Resources/PressStart2P-Regular.ttf)

target_sources(CHIPCORE PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_include_directories(CHIPCORE PRIVATE Source)

target_compile_definitions(CHIPCORE PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_REPORT_APP_USAGE=0)

# Lets the project compile on minimal Linux boxes (used only for CI sanity builds)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CHIPCORE_MINIMAL_X11)
    target_compile_definitions(CHIPCORE PUBLIC
        JUCE_USE_XRANDR=0 JUCE_USE_XINERAMA=0 JUCE_USE_XRENDER=0 JUCE_USE_XCURSOR=0
        JUCE_ALSA=0 JUCE_JACK=0)
endif()

option(CHIPCORE_BUILD_TESTS "Build the developer host test tool" OFF)
if(CHIPCORE_BUILD_TESTS)
    juce_add_console_app(ChipcoreHostTest PRODUCT_NAME "ChipcoreHostTest")
    target_sources(ChipcoreHostTest PRIVATE
        tests/host/HostTest.cpp
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)
    target_include_directories(ChipcoreHostTest PRIVATE Source)
    target_compile_definitions(ChipcoreHostTest PRIVATE
        JUCE_PLUGINHOST_VST3=1 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
        JucePlugin_Name="CHIPCORE")
    target_link_libraries(ChipcoreHostTest PRIVATE
        ChipcoreData juce::juce_audio_processors juce::juce_audio_formats juce::juce_gui_extra
        juce::juce_recommended_config_flags)
endif()

target_link_libraries(CHIPCORE
    PRIVATE
        ChipcoreData
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
