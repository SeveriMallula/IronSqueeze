#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace juce;
AudioProcessorValueTreeState::ParameterLayout SoundImagerProcessor::layout() {
 AudioProcessorValueTreeState::ParameterLayout p;
 auto add=[&](const char* id,const char* name,float lo,float hi,float def){p.add(std::make_unique<AudioParameterFloat>(ParameterID{id,1},name,NormalisableRange<float>{lo,hi,0.01f},def));};
 add("width","Width (%)",0,300,100); add("rotation","Rotation (degrees)",-45,45,0);
 add("asymmetry","Asymmetry (dB)",-12,12,0); add("balance","Balance (%)",-100,100,0);
 add("gain","Output (dB)",-24,12,0); add("bass","Mono bass (Hz; 0 = off)",0,500,0);
 for(auto id:{"mono","swap","bypass"}) p.add(std::make_unique<AudioParameterBool>(ParameterID{id,1},String(id).toUpperCase(),false));
 return p;
}
SoundImagerProcessor::SoundImagerProcessor():AudioProcessor(BusesProperties().withInput("Input",AudioChannelSet::stereo(),true).withOutput("Output",AudioChannelSet::stereo(),true)),state(*this,nullptr,"SoundImagerState",layout()) {}
soundimager::Settings SoundImagerProcessor::settings() const {
 auto v=[&](const char* id){return state.getRawParameterValue(id)->load();};
 return {v("width")/100,v("rotation"),v("asymmetry"),v("balance")/100,v("gain"),v("bass"),v("mono"),v("swap"),v("bypass")};
}
void SoundImagerProcessor::prepareToPlay(double sr,int) {core.prepare(sr,settings()); ll=rr=lr=0; meterDecay=std::exp(-1/(sr*0.15)); correlation=peakL=peakR=0;}
bool SoundImagerProcessor::isBusesLayoutSupported(const BusesLayout& b) const {return b.getMainInputChannelSet()==AudioChannelSet::stereo() && b.getMainOutputChannelSet()==AudioChannelSet::stereo();}
void SoundImagerProcessor::processBlock(AudioBuffer<float>& b,MidiBuffer&) {run(b,false);}
void SoundImagerProcessor::processBlockBypassed(AudioBuffer<float>& b,MidiBuffer&) {run(b,true);}
void SoundImagerProcessor::run(AudioBuffer<float>& b,bool hostBypass) {
 ScopedNoDenormals noDenormals;
 if(b.getNumChannels()<2) return;
 auto s=settings(); if(hostBypass) s.bypass=1; core.set(s);
 auto* l=b.getWritePointer(0); auto* r=b.getWritePointer(1);
 float pl=peakL.load(),pr=peakR.load();
 const float pd=float(std::pow(meterDecay,0.3));
 for(int i=0;i<b.getNumSamples();++i) {
  core.process(l[i],r[i]);
  ll=meterDecay*ll+(1-meterDecay)*l[i]*l[i]; rr=meterDecay*rr+(1-meterDecay)*r[i]*r[i]; lr=meterDecay*lr+(1-meterDecay)*l[i]*r[i];
  pl=std::max(std::abs(l[i]),pl*pd); pr=std::max(std::abs(r[i]),pr*pd);
  if(i%32==0) {int a,n,c,k;scopeFifo.prepareToWrite(1,a,n,c,k);if(n>0){scope[size_t(a)]={l[i],r[i]};scopeFifo.finishedWrite(1);}}
 }
 peakL=pl;peakR=pr;correlation=float(std::clamp(lr/std::sqrt(std::max(1e-30,ll*rr)),-1.0,1.0));
}
void SoundImagerProcessor::getStateInformation(MemoryBlock& b){auto xml=state.copyState().createXml();copyXmlToBinary(*xml,b);}
void SoundImagerProcessor::setStateInformation(const void* d,int n){auto xml=getXmlFromBinary(d,n);if(xml && xml->hasTagName(state.state.getType()))state.replaceState(ValueTree::fromXml(*xml));}
AudioProcessorEditor* SoundImagerProcessor::createEditor(){return new SoundImagerEditor(*this);}
AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SoundImagerProcessor();}
