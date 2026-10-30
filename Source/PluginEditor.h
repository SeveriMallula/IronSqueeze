#pragma once
#include "PluginProcessor.h"
class RackLook : public juce::LookAndFeel_V4 {
public:
    RackLook();
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
};
class IronEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit IronEditor(IronProcessor&);
    ~IronEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    IronProcessor& proc;RackLook look;
    std::array<juce::Slider,5> knobs;
    std::array<juce::Label,5> labels;
    std::array<juce::TextButton,8> ratio;
    std::array<juce::TextButton,3> colour;
    std::array<juce::ToggleButton,6> toggles;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliders;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttons;
    juce::ComboBox presets;
    void preset(int);
    float displayedGR=0;
};
