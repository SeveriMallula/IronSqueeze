#include "../Source/PluginProcessor.h"
#include <iostream>
#include <stdexcept>
using namespace juce;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static void set(IronProcessor& p,const char* id,float v){auto* q=p.state.getParameter(id);q->setValueNotifyingHost(q->convertTo0to1(v));}
int main(int argc,char** argv){try{
    ScopedJuceInitialiser_GUI init;
    IronProcessor p;set(p,"ratio",0);p.prepareToPlay(48000,128);
    const int latency=p.getLatencySamples();require(latency>0,"oversampling latency reported");
    MidiBuffer midi;AudioBuffer<float> b(2,997); // larger than host's announced block
    b.clear();p.processBlockBypassed(b,midi); // finish the deliberate bypass crossfade
    b.clear();b.setSample(0,0,.5f);b.setSample(1,0,-.25f);
    p.processBlockBypassed(b,midi);
    for(int i=0;i<b.getNumSamples();++i){require(std::abs(b.getSample(0,i)-(i==latency?.5f:0))<1e-6f,"bypass delay alignment");require(std::isfinite(b.getSample(1,i)),"finite bypass");}
    std::cout<<"PASS host bypass delay and oversized block; latency "<<latency<<" samples\n";
    set(p,"input",11.25f);set(p,"ratio",5);set(p,"detHP",1);set(p,"colour",2);
    MemoryBlock state;p.getStateInformation(state);IronProcessor restored;restored.setStateInformation(state.getData(),(int)state.getSize());
    for(const char* id:{"input","ratio","detHP","colour"})require(std::abs(restored.state.getRawParameterValue(id)->load()-p.state.getRawParameterValue(id)->load())<1e-4f,"state roundtrip");
    std::cout<<"PASS state roundtrip\n";
    restored.prepareToPlay(44100,64);
    for(int k=0;k<100;++k){AudioBuffer<float> audio(2,257);for(int c=0;c<2;++c)for(int i=0;i<257;++i)audio.setSample(c,i,.3f*std::sin((float)(k*257+i)*.13f));restored.processBlock(audio,midi);for(int c=0;c<2;++c)for(int i=0;i<257;++i)require(std::isfinite(audio.getSample(c,i)),"processor finite");}
    IronProcessor unity;set(unity,"ratio",0);set(unity,"mix",50);unity.prepareToPlay(48000,128);
    double err=0,power=0;
    for(int block=0;block<400;++block){AudioBuffer<float> tone(2,128);
        for(int i=0;i<128;++i){float x=.3f*std::sin((float)(block*128+i)*2.f*MathConstants<float>::pi*1000.f/48000.f);tone.setSample(0,i,x);tone.setSample(1,i,x);}
        unity.processBlock(tone,midi);
        if(block>100)for(int i=0;i<128;++i){double expected=.3*std::sin((block*128+i-unity.getLatencySamples())*2.*MathConstants<double>::pi*1000./48000.);err+=std::pow(tone.getSample(0,i)-expected,2);power+=expected*expected;}}
    require(std::sqrt(err/power)<.003,"linear phase parallel unity");std::cout<<"PASS linear phase parallel alignment at 1 kHz\n";
    // Dry mix must be exactly a delayed input, independent of wet state.
    set(restored,"mix",0);restored.prepareToPlay(96000,64);
    AudioBuffer<float> dry(2,1024);dry.clear();dry.setSample(0,0,.75f);restored.processBlock(dry,midi);
    for(int i=0;i<1024;++i)require(std::abs(dry.getSample(0,i)-(i==restored.getLatencySamples()?.75f:0))<1e-6f,"zero mix dry alignment");
    AudioProcessor::BusesLayout mono;mono.inputBuses.add(AudioChannelSet::mono());mono.outputBuses.add(AudioChannelSet::mono());
    require(restored.setBusesLayout(mono),"mono layout");restored.prepareToPlay(192000,32);AudioBuffer<float> one(1,129);one.clear();restored.processBlock(one,midi);
    std::cout<<"PASS dry mix delay, variable blocks, mono/stereo and sample rate reprepare\nALL PROCESSOR TESTS PASSED\n";
    if(argc==3 && String(argv[1])=="--render"){
        IronProcessor preview;set(preview,"input",9);set(preview,"colour",1);set(preview,"detHP",1);
        std::unique_ptr<AudioProcessorEditor> editor(preview.createEditor());
        Image image(Image::ARGB,editor->getWidth(),editor->getHeight(),true);Graphics g(image);editor->paintEntireComponent(g,true);
        File file=File::getCurrentWorkingDirectory().getChildFile(argv[2]);FileOutputStream stream(file);PNGImageFormat png;require(png.writeImageToStream(image,stream),"UI render");
        std::cout<<"PASS actual JUCE editor render\n";
    }
    return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
