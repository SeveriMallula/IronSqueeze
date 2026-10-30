#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace juce;
AudioProcessorValueTreeState::ParameterLayout IronProcessor::layout() {
    AudioProcessorValueTreeState::ParameterLayout l;
    auto f=[&](const char* id,const char* name,float lo,float hi,float def,float skew=1.f){
        NormalisableRange<float> range(lo,hi,.001f,skew);
        l.add(std::make_unique<AudioParameterFloat>(ParameterID{id,1},name,range,def));
    };
    f("input","Input dB",-24,36,0);f("attack","Attack ms",.05f,30,5,.35f);
    f("release","Release ms",50,3500,150,.35f);f("output","Output dB",-36,24,0);
    l.add(std::make_unique<AudioParameterChoice>(ParameterID{"ratio",1},"Ratio",
        StringArray{"1:1","2:1","3:1","4:1","6:1","10:1 OPT","20:1","CRUSH"},4));
    l.add(std::make_unique<AudioParameterChoice>(ParameterID{"colour",1},"Harmonics",
        StringArray{"Clean","H2","H3"},0));
    for(auto item: {std::pair<const char*,const char*>{"detHP","Detector HP"},{"emphasis","Detector 6 kHz"},
                   {"audioHP","Audio HP"},{"link","Stereo link"},{"british","British curve"},{"bypass","Bypass"}})
        l.add(std::make_unique<AudioParameterBool>(ParameterID{item.first,1},item.second,String(item.first)=="link"));
    f("mix","Mix percent",0,100,100);
    return l;
}
IronProcessor::IronProcessor():AudioProcessor(BusesProperties().withInput("Input",AudioChannelSet::stereo(),true)
    .withOutput("Output",AudioChannelSet::stereo(),true)),state(*this,nullptr,"IronSqueeze",layout()) {
    const char* ids[]={"input","attack","release","output","ratio","colour","detHP","emphasis","audioHP","link","british","bypass","mix"};
    for(size_t i=0;i<values.size();++i)values[i]=state.getRawParameterValue(ids[i]);
}
bool IronProcessor::isBusesLayoutSupported(const BusesLayout& l)const {
    return (l.getMainOutputChannelSet()==AudioChannelSet::mono()||l.getMainOutputChannelSet()==AudioChannelSet::stereo())
        && l.getMainInputChannelSet()==l.getMainOutputChannelSet();
}
void IronProcessor::prepareToPlay(double fs,int maxBlock) {
    chunkSize=jmax(32,maxBlock); const auto n=(size_t)getTotalNumInputChannels();
    oversampling=std::make_unique<dsp::Oversampling<float>>(n,2,dsp::Oversampling<float>::filterHalfBandFIREquiripple,true,true);
    oversampling->initProcessing((size_t)chunkSize);
    latency=(int)std::lround(oversampling->getLatencyInSamples());setLatencySamples(latency);
    dry.setSize((int)n,chunkSize);
    for(auto& d:delay)d.assign((size_t)jmax(1,latency),0);
    delayPos=0;core.prepare(fs*4);
    mix.reset(fs,.02);mix.setCurrentAndTargetValue(values[12]->load()*.01f);
    engaged.reset(fs,.01);engaged.setCurrentAndTargetValue(values[11]->load()>.5f?0.f:1.f);
}
void IronProcessor::processBlock(AudioBuffer<float>& b,MidiBuffer&){render(b,false);}
void IronProcessor::processBlockBypassed(AudioBuffer<float>& b,MidiBuffer&){render(b,true);}
void IronProcessor::render(AudioBuffer<float>& b,bool hostBypass) {
    ScopedNoDenormals guard;
    for(int c=getTotalNumInputChannels();c<b.getNumChannels();++c)b.clear(c,0,b.getNumSamples());
    if(!oversampling)return;
    iron::Parameters p;
    p.inputDB=values[0]->load();p.attackMS=values[1]->load();p.releaseMS=values[2]->load();p.outputDB=values[3]->load();
    p.ratio=(int)values[4]->load();p.colour=(int)values[5]->load();p.detectorHP=values[6]->load()>.5f;
    p.emphasis=values[7]->load()>.5f;p.audioHP=values[8]->load()>.5f;p.linked=values[9]->load()>.5f;
    p.british=values[10]->load()>.5f;core.set(p);
    mix.setTargetValue(values[12]->load()*.01f);engaged.setTargetValue(hostBypass||values[11]->load()>.5f?0.f:1.f);
    const int channels=getTotalNumInputChannels();
    float maxGR=0,maxIn=0,maxOut=0;
    for(int offset=0;offset<b.getNumSamples();offset+=chunkSize) {
        const int count=jmin(chunkSize,b.getNumSamples()-offset);
        for(int i=0;i<count;++i) {
            for(int c=0;c<channels;++c) {
                float x=b.getSample(c,offset+i);if(!std::isfinite(x))x=0;
                maxIn=jmax(maxIn,std::abs(x));
                auto& d=delay[(size_t)c];const float delayed=latency?d[(size_t)delayPos]:x;
                if(latency)d[(size_t)delayPos]=x;
                dry.setSample(c,i,delayed); b.setSample(c,offset+i,x);
            }
            if(latency)delayPos=(delayPos+1)%latency;
        }
        auto block=dsp::AudioBlock<float>(b).getSubBlock((size_t)offset,(size_t)count);
        auto up=oversampling->processSamplesUp(block);
        auto* l=up.getChannelPointer(0);auto* r=channels>1?up.getChannelPointer(1):nullptr;
        for(size_t i=0;i<up.getNumSamples();++i){auto y=core.tick(l[i],r?r[i]:l[i],channels);l[i]=y[0];if(r)r[i]=y[1];maxGR=jmax(maxGR,(float)core.reduction);}
        oversampling->processSamplesDown(block);
        for(int i=0;i<count;++i) {
            const float wet=mix.getNextValue()*engaged.getNextValue();
            for(int c=0;c<channels;++c){float x= dry.getSample(c,i);const float y=x+wet*(b.getSample(c,offset+i)-x);b.setSample(c,offset+i,y);maxOut=jmax(maxOut,std::abs(y));}
        }
    }
    gr.store(maxGR);inputPeak.store(Decibels::gainToDecibels(maxIn,-100.f));outputPeak.store(Decibels::gainToDecibels(maxOut,-100.f));
}
AudioProcessorParameter* IronProcessor::getBypassParameter()const{return state.getParameter("bypass");}
void IronProcessor::getStateInformation(MemoryBlock& d){auto xml=state.copyState().createXml();copyXmlToBinary(*xml,d);}
void IronProcessor::setStateInformation(const void* d,int n){if(auto xml=getXmlFromBinary(d,n))if(xml->hasTagName(state.state.getType()))state.replaceState(ValueTree::fromXml(*xml));}
AudioProcessorEditor* IronProcessor::createEditor(){return new IronEditor(*this);}
AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new IronProcessor();}
