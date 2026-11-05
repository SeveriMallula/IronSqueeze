cmake_minimum_required(VERSION 3.22)
# Set before project() initializes the Apple toolchain and deployment target.
if(APPLE OR CMAKE_HOST_APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS" FORCE)
endif()
project(SoundImager VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
option(IMAGER_CORE_ONLY "Build portable DSP tests without JUCE" OFF)
enable_testing()
add_executable(imager_core_tests Tests/CoreTests.cpp)
add_test(NAME DSP COMMAND imager_core_tests)
if(IMAGER_CORE_ONLY)
    return()
endif()
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS")
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/JUCE/CMakeLists.txt")
    add_subdirectory(../third_party/JUCE JUCE)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/JUCE/CMakeLists.txt")
    add_subdirectory(third_party/JUCE JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 29396c22c93392d6738e021b83196283d6e4d850 GIT_SHALLOW FALSE)
    FetchContent_MakeAvailable(JUCE)
endif()
set(IRON_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND IRON_FORMATS AU)
endif()
juce_add_plugin(SoundImager COMPANY_NAME "Mallula Audio" BUNDLE_ID "audio.mallula.soundimager"
    PLUGIN_MANUFACTURER_CODE Mllu PLUGIN_CODE SoIm FORMATS ${IRON_FORMATS}
    PRODUCT_NAME "Sound Imager"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Sound Imager uses audio input for standalone stereo processing."
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE COPY_PLUGIN_AFTER_BUILD FALSE VST3_CATEGORIES Fx Spatial
    AU_MAIN_TYPE kAudioUnitType_Effect)
target_sources(SoundImager PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(SoundImager PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(SoundImager PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)


add_executable(imager_processor_tests Tests/ProcessorTests.cpp)
target_link_libraries(imager_processor_tests PRIVATE SoundImager)
target_include_directories(imager_processor_tests PRIVATE "$<TARGET_PROPERTY:SoundImager,INCLUDE_DIRECTORIES>")
target_compile_definitions(imager_processor_tests PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
add_test(NAME Processor COMMAND imager_processor_tests)
