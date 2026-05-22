cmake_minimum_required(VERSION 3.22)
project(MyCompressor VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)

# --- Récupération de JUCE 8 via FetchContent ---
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(MyCompressor
    COMPANY_NAME "MyCompany"
    PLUGIN_MANUFACTURER_CODE "Mcmp"
    PLUGIN_CODE "Mcmp"
    FORMATS VST3 Standalone
    PRODUCT_NAME "My Compressor"
)

target_sources(MyCompressor
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/CompressorEngine.cpp
)

target_include_directories(MyCompressor PRIVATE Source)

target_link_libraries(MyCompressor
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(MyCompressor
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)