#include "PluginEditor.h"
using namespace juce;
static const Colour cream(0xfff3e5c9),amber(0xffffb548),panel(0xff171b1d);
RackLook::RackLook(){
    setColour(Slider::textBoxTextColourId,cream);setColour(Slider::textBoxOutlineColourId,Colours::transparentBlack);
    setColour(Slider::textBoxBackgroundColourId,Colour(0xff101314));
    setColour(TextButton::buttonColourId,Colour(0xff2e3436));setColour(TextButton::buttonOnColourId,amber);
    setColour(TextButton::textColourOffId,cream);setColour(TextButton::textColourOnId,Colours::black);
    setColour(ToggleButton::textColourId,cream);setColour(ToggleButton::tickColourId,amber);
    setColour(ComboBox::backgroundColourId,Colour(0xff252b2d));setColour(ComboBox::textColourId,cream);
}
void RackLook::drawRotarySlider(Graphics& g,int x,int y,int w,int h,float pos,float start,float end,Slider&) {
    auto b=Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(12);const auto r=jmin(b.getWidth(),b.getHeight())*.5f;
    const auto centre=b.getCentre();b={centre.x-r,centre.y-r,r*2,r*2};
    g.setColour(Colours::black.withAlpha(.6f));g.fillEllipse(b.translated(2,5));
    Path ring;ring.addCentredArc(centre.x,centre.y,r+5,r+5,0,start,end,true);
    g.setColour(Colour(0xff43494b));g.strokePath(ring,PathStrokeType(2));
    Path lit;lit.addCentredArc(centre.x,centre.y,r+5,r+5,0,start,start+pos*(end-start),true);
    g.setColour(amber);g.strokePath(lit,PathStrokeType(3));
    g.setGradientFill(ColourGradient(Colour(0xfffff7e5),centre.x,centre.y-r,Colour(0xffaa9e88),centre.x,centre.y+r,false));g.fillEllipse(b);
    g.setColour(cream.withAlpha(.5f));g.drawEllipse(b.reduced(3),1);
    Path pointer;pointer.addRoundedRectangle(-2,-r+9,4,r*.47f,2);
    g.setColour(Colour(0xff252a2c));g.fillPath(pointer,AffineTransform::rotation(start+pos*(end-start)).translated(centre));
}
IronEditor::IronEditor(IronProcessor& p):AudioProcessorEditor(p),proc(p){
    setLookAndFeel(&look);setSize(1060,440);setResizable(false,false);
    const char* ids[]={"input","attack","release","output","mix"};
    const char* names[]={"INPUT","ATTACK","RELEASE","OUTPUT","PARALLEL MIX"};
    const char* units[]={" dB"," ms"," ms"," dB"," %"};
    for(size_t i=0;i<knobs.size();++i){auto& k=knobs[i];k.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);k.setTextBoxStyle(Slider::TextBoxBelow,false,112,24);k.setTextValueSuffix(units[i]);k.setNumDecimalPlacesToDisplay(i==1?2:1);addAndMakeVisible(k);
        labels[i].setText(names[i],dontSendNotification);labels[i].setJustificationType(Justification::centred);labels[i].setColour(Label::textColourId,cream);addAndMakeVisible(labels[i]);
        sliders.push_back(std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(p.state,ids[i],k));}
    const StringArray ratios{"1:1","2:1","3:1","4:1","6:1","10:1 OPT","20:1","CRUSH"};
    for(size_t i=0;i<ratio.size();++i){auto& b=ratio[i];b.setButtonText(ratios[(int)i]);addAndMakeVisible(b);b.onClick=[this,i]{auto* par=proc.state.getParameter("ratio");par->beginChangeGesture();par->setValueNotifyingHost(par->convertTo0to1((float)i));par->endChangeGesture();};}
    for(size_t i=0;i<colour.size();++i){auto& b=colour[i];b.setButtonText(StringArray{"CLEAN","H2","H3"}[(int)i]);addAndMakeVisible(b);b.onClick=[this,i]{auto* par=proc.state.getParameter("colour");par->beginChangeGesture();par->setValueNotifyingHost(par->convertTo0to1((float)i));par->endChangeGesture();};}
    const char* toggleIDs[]={"detHP","emphasis","audioHP","link","british","bypass"};
    const char* toggleNames[]={"SC LOW CUT","SC 6 kHz","AUDIO 80 Hz","STEREO LINK","BRITISH","BYPASS"};
    for(size_t i=0;i<toggles.size();++i){toggles[i].setButtonText(toggleNames[i]);addAndMakeVisible(toggles[i]);buttons.push_back(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(p.state,toggleIDs[i],toggles[i]));}
    presets.addItemList({"Neutral start","Vocal control","Optical bass","Room crush","Gentle mix bus"},1);presets.setText("Starting points");presets.onChange=[this]{preset(presets.getSelectedId());};addAndMakeVisible(presets);
    knobs[0].setTooltip("Drive the fixed threshold harder for more compression.");
    knobs[4].setTooltip("Blend latency-aligned dry audio with the processed signal.");
    timerCallback();
    startTimerHz(30);
}
IronEditor::~IronEditor(){stopTimer();setLookAndFeel(nullptr);}
void IronEditor::resized(){
    presets.setBounds(805,22,220,28);
    for(int i=0;i<5;++i){int x=25+i*204;labels[(size_t)i].setBounds(x,163,194,22);knobs[(size_t)i].setBounds(x+15,184,164,152);}
    for(int i=0;i<8;++i)ratio[(size_t)i].setBounds(28+i*92,119,86,27);
    for(int i=0;i<3;++i)colour[(size_t)i].setBounds(794+i*77,119,70,27);
    for(int i=0;i<6;++i)toggles[(size_t)i].setBounds(29+i*170,357,165,30);
}
void IronEditor::paint(Graphics& g){
    g.fillAll(panel);g.setColour(Colour(0xff303638));g.drawRect(getLocalBounds(),2);
    g.setColour(cream);g.setFont(26.f);g.drawText("IRON SQUEEZE",28,20,300,32,Justification::centredLeft);
    g.setFont(11.f);g.setColour(Colour(0xffa3acae));g.drawText("MALLULA AUDIO  /  FEEDBACK CHARACTER COMPRESSOR",29,53,500,18,Justification::centredLeft);
    g.setColour(Colour(0xff090c0d));g.fillRoundedRectangle(350,20,424,78,5);
    static const float steps[]={1,2,3,4,5,6,8,10,12,15,18,21,24,30};
    g.setFont(10.f);
    for(int i=0;i<14;++i){const bool on=displayedGR>=steps[i];g.setColour((i>10?Colour(0xffef6659):amber).withAlpha(on?1.f:.12f));g.fillRoundedRectangle((float)(365+i*22),41,15,15,2);g.setColour(Colour(0xff839092));g.drawText(String(steps[i],0),360+i*22,62,25,14,Justification::centred);}
    g.setColour(cream);g.setFont(17.f);g.drawText(String(displayedGR,1),686,33,70,24,Justification::centred);
    g.setFont(10.f);g.drawText("dB REDUCTION",673,65,92,16,Justification::centred);
    g.setColour(Colour(0xff697679));g.drawHorizontalLine(106,28,1030);g.drawHorizontalLine(347,28,1030);
    g.setColour(Colour(0xffb2bcbe));g.setFont(11.f);
    const float out=proc.outputPeak.load();g.drawText("IN  "+String(proc.inputPeak.load(),1)+" dBFS     OUT  "+String(out,1)+" dBFS",28,405,440,18,Justification::centredLeft);
    if(out>0){g.setColour(Colour(0xffff6b59));g.drawText("OUTPUT OVER 0 dBFS",420,405,200,18,Justification::centred);}
    g.setColour(Colour(0xff879295));g.drawText("4x LINEAR PHASE   /   v0.2.1",700,405,330,18,Justification::centredRight);
}
void IronEditor::timerCallback(){displayedGR=jmax(proc.gr.load(),displayedGR*.88f);
    for(int i=0;i<8;++i)ratio[(size_t)i].setToggleState(i==(int)proc.state.getRawParameterValue("ratio")->load(),dontSendNotification);
    for(int i=0;i<3;++i)colour[(size_t)i].setToggleState(i==(int)proc.state.getRawParameterValue("colour")->load(),dontSendNotification);
    repaint();
}
void IronEditor::preset(int id){
    struct Start {float input,attack,release,output,ratio,colour,mix,hp,brit;};
    const Start starts[]={{0,5,150,0,4,0,100,0,0},{9,2,180,-3,4,1,100,1,0},
        {8,30,70,-3,5,1,100,1,0},{20,8,80,-6,7,2,55,1,0},{2,20,300,0,1,0,100,1,0}};
    if(id<1||id>5)return;
    const auto& s=starts[id-1];
    const char* ids[]={"input","attack","release","output","ratio","colour","mix","detHP","british","emphasis","audioHP","link","bypass"};
    const float v[]={s.input,s.attack,s.release,s.output,s.ratio,s.colour,s.mix,s.hp,s.brit,0,0,1,0};
    for(int i=0;i<13;++i){auto* p=proc.state.getParameter(ids[i]);p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(v[i]));p->endChangeGesture();}
}
