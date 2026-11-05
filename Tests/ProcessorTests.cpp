#include "../Source/PluginProcessor.h"
#include <iostream>
#include <stdexcept>
void check(bool c,const char* s){if(!c)throw std::runtime_error(s);}
int main(int argc,char** argv){try {
 juce::ScopedJuceInitialiser_GUI init;
 SoundImagerProcessor p;p.prepareToPlay(48000,512);
 juce::AudioBuffer<float> b(2,512);juce::MidiBuffer midi;
 for(int i=0;i<512;++i){b.setSample(0,i,.2f);b.setSample(1,i,-.1f);}p.processBlock(b,midi);
 check(std::abs(b.getSample(0,100)-.2f)<1e-6f,"unity L");check(std::abs(b.getSample(1,100)+.1f)<1e-6f,"unity R");
 auto* width=p.state.getParameter("width");width->setValueNotifyingHost(width->convertTo0to1(234));juce::MemoryBlock saved;p.getStateInformation(saved);
 SoundImagerProcessor q;q.setStateInformation(saved.getData(),int(saved.getSize()));check(std::abs(q.state.getRawParameterValue("width")->load()-234)<.01f,"state restore");
 for(int k=0;k<20;++k){b.clear();p.processBlock(b,midi);}check(p.getLatencySamples()==0,"zero latency");
 juce::AudioProcessor::BusesLayout mono;mono.inputBuses.add(juce::AudioChannelSet::mono());mono.outputBuses.add(juce::AudioChannelSet::mono());check(!p.isBusesLayoutSupported(mono),"reject mono bus");
 std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());check(editor->getWidth()>=880,"editor construction");
 if(argc==2){juce::Image image(juce::Image::ARGB,editor->getWidth(),editor->getHeight(),true);juce::Graphics g(image);editor->paintEntireComponent(g,true);juce::FileOutputStream stream(juce::File::getCurrentWorkingDirectory().getChildFile(argv[1]));juce::PNGImageFormat png;check(png.writeImageToStream(image,stream),"render editor");}
 std::cout<<"PASS: processor, state restore, stereo layout, latency, editor\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
