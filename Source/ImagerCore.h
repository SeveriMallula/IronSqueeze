#pragma once
#include <algorithm>
#include <cmath>
namespace soundimager {
struct Settings { float width=1, rotation=0, asymmetry=0, balance=0, gain=0, bass=0, mono=0, swap=0, bypass=0; };
class Core {
    Settings current, target;
    double sr=48000;
    float low=0, smoothing=1;
public:
    void prepare(double rate, Settings initial={}) { sr=rate; current=target=initial; low=0; smoothing=float(1-std::exp(-1/(0.02*sr))); }
    void set(Settings s) { target=s; }
    void process(float& l,float& r) {
        const float dryL=l,dryR=r;
#define SMOOTH(x) current.x += smoothing*(target.x-current.x)
        SMOOTH(width); SMOOTH(rotation); SMOOTH(asymmetry); SMOOTH(balance); SMOOTH(gain); SMOOTH(bass); SMOOTH(mono); SMOOTH(swap); SMOOTH(bypass);
#undef SMOOTH
        float m=(l+r)*0.5f, s=(l-r)*0.5f;
        const float a=float(1-std::exp(-6.283185307179586*std::max(20.0f,current.bass)/sr));
        low+=a*(s-low);
        const float bassMix=std::clamp(current.bass/20.0f,0.0f,1.0f);
        s=(s-low*bassMix)*current.width*(1-current.mono);
        l=(m+s)*std::pow(10.0f,current.asymmetry/40.0f);
        r=(m-s)*std::pow(10.0f,-current.asymmetry/40.0f);
        const float angle=current.rotation*0.017453292519943f;
        float x=l*std::cos(angle)-r*std::sin(angle), y=l*std::sin(angle)+r*std::cos(angle);
        const float g=std::pow(10.0f,current.gain/20.0f);
        x*=g*(1-std::max(0.0f,current.balance)); y*=g*(1+std::min(0.0f,current.balance));
        l=x+(y-x)*current.swap; r=y+(x-y)*current.swap;
        l+=(dryL-l)*current.bypass; r+=(dryR-r)*current.bypass;
    }
};
}
