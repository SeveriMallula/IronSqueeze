#pragma once
#include "PluginProcessor.h"
class SoundImagerEditor : public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit SoundImagerEditor(SoundImagerProcessor&);
 ~SoundImagerEditor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 SoundImagerProcessor& processor;
 juce::LookAndFeel_V4 theme;
 std::array<juce::Slider,6> knobs;
 std::array<juce::Label,6> labels;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,6> attachments;
 std::array<juce::ToggleButton,3> buttons;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>,3> buttonAttachments;
 juce::TextButton reset{"Reset"};
 std::array<juce::Point<float>,512> points{};
 int head=0;
 void timerCallback() override;
};
