cmake_minimum_required(VERSION 3.22)
# Set before project() initializes the Apple toolchain and deployment target.
if(APPLE OR CMAKE_HOST_APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS" FORCE)
endif()
project(IronSqueeze VERSION 0.2.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
option(IRON_CORE_ONLY "Build portable DSP tests without JUCE" OFF)
enable_testing()
add_executable(iron_core_tests Tests/CoreTests.cpp)
add_test(NAME DSP COMMAND iron_core_tests)
if(IRON_CORE_ONLY)
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
juce_add_plugin(IronSqueeze COMPANY_NAME "Mallula Audio" BUNDLE_ID "audio.mallula.ironsqueeze"
    PLUGIN_MANUFACTURER_CODE Mllu PLUGIN_CODE IrSq FORMATS ${IRON_FORMATS}
    PRODUCT_NAME "Iron Squeeze"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Iron Squeeze uses audio input for standalone compression."
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE COPY_PLUGIN_AFTER_BUILD FALSE VST3_CATEGORIES Fx Dynamics
    AU_MAIN_TYPE kAudioUnitType_Effect)
target_sources(IronSqueeze PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(IronSqueeze PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(IronSqueeze PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Host-independent processor integration checks, linked to the real plugin code.
add_executable(iron_processor_tests Tests/ProcessorTests.cpp)
target_link_libraries(iron_processor_tests PRIVATE IronSqueeze)
target_include_directories(iron_processor_tests PRIVATE "$<TARGET_PROPERTY:IronSqueeze,INCLUDE_DIRECTORIES>")
target_compile_definitions(iron_processor_tests PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
add_test(NAME Processor COMMAND iron_processor_tests)
if(WIN32)
    install(DIRECTORY "${CMAKE_BINARY_DIR}/IronSqueeze_artefacts/$<CONFIG>/VST3/Iron Squeeze.vst3"
        DESTINATION "Common Files/VST3" COMPONENT VST3)
endif()
if(UNIX AND NOT APPLE)
    add_executable(iron_vst3_smoke Tests/VST3Smoke.cpp)
    target_include_directories(iron_vst3_smoke PRIVATE "$<TARGET_PROPERTY:IronSqueeze,INCLUDE_DIRECTORIES>")
    target_link_libraries(iron_vst3_smoke PRIVATE ${CMAKE_DL_LIBS})
    add_dependencies(iron_vst3_smoke IronSqueeze_VST3)
    add_test(NAME VST3Module COMMAND iron_vst3_smoke "$<TARGET_FILE:IronSqueeze_VST3>")
endif()
