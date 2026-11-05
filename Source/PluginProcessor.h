#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "ImagerCore.h"
class SoundImagerProcessor : public juce::AudioProcessor {
public:
 SoundImagerProcessor();
 juce::AudioProcessorValueTreeState state;
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
 soundimager::Settings settings() const;
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override { return true; }
 const juce::String getName() const override { return "Sound Imager"; }
 bool acceptsMidi() const override { return false; }
 bool producesMidi() const override { return false; }
 double getTailLengthSeconds() const override { return 0; }
 int getNumPrograms() override { return 1; }
 int getCurrentProgram() override { return 0; }
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override { return {}; }
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 juce::AudioProcessorParameter* getBypassParameter() const override { return state.getParameter("bypass"); }
 std::atomic<float> correlation{0},peakL{0},peakR{0};
 juce::AbstractFifo scopeFifo{2048};
 std::array<juce::Point<float>,2048> scope{};
private:
 soundimager::Core core;
 double ll=0,rr=0,lr=0,meterDecay=0.999;
 void run(juce::AudioBuffer<float>&,bool);
};
