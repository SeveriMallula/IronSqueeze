#pragma once
#include <algorithm>
#include <array>
#include <cmath>

namespace iron {
constexpr double pi = 3.14159265358979323846;
inline double db(double x) { return 20.0 * std::log10(std::max(1.0e-12, std::abs(x))); }
inline double gain(double x) { return std::pow(10.0, x / 20.0); }
struct Parameters {
    double inputDB = 0, outputDB = 0, attackMS = 5, releaseMS = 150;
    int ratio = 4, colour = 0;
    bool detectorHP = false, emphasis = false, audioHP = false, linked = true, british = false;
};
struct Mode { double ratio, threshold, knee; };
// Thresholds/knees are design estimates, NOT extracted component values.
inline constexpr std::array<Mode,8> modes {{{1,-18,0},{2,-22,15},{3,-21,12},
    {4,-20,9},{6,-19,6},{10,-18,4},{20,-17,2},{40,-22,0}}};
struct Biquad {
    double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
    double tick(double x) {
        const double y=b0*x+z1; z1=b1*x-a1*y+z2; z2=b2*x-a2*y;
        if (std::abs(z1)<1e-24) z1=0;
        if (std::abs(z2)<1e-24) z2=0;
        return y;
    }
    void hp(double fs,double hz) {
        const double w=2*pi*hz/fs,c=std::cos(w),al=std::sin(w)/std::sqrt(2.0),a0=1+al;
        b0=(1+c)*.5/a0;b1=-(1+c)/a0;b2=b0;a1=-2*c/a0;a2=(1-al)/a0;
    }
    void bell(double fs,double hz,double boost,double q) {
        const double w=2*pi*std::min(hz,fs*.4)/fs,A=std::pow(10.,boost/40.),
            al=std::sin(w)/(2*q),c=std::cos(w),a0=1+al/A;
        b0=(1+al*A)/a0;b1=-2*c/a0;b2=(1-al*A)/a0;a1=b1;a2=(1-al/A)/a0;
    }
};
class Core {
public:
    void prepare(double sampleRate) {
        fs=sampleRate; smooth=1-std::exp(-1/(fs*.015));
        for(auto& s:ch) {
            s={};s.hp.hp(fs,80);s.sc.hp(fs,90);s.eq.bell(fs,6000,8,1.0);s.dc.hp(fs,5);
        }
        in=out=1; hpMix=scMix=eqMix=0; c2=c3=0; reduction=0; peakIn=peakOut=0;
    }
    void set(const Parameters& value) {
        p=value;p.ratio=std::clamp(p.ratio,0,7);p.colour=std::clamp(p.colour,0,2);
        p.attackMS=std::clamp(p.attackMS,.05,30.);p.releaseMS=std::clamp(p.releaseMS,50.,3500.);
        inputTarget=gain(std::clamp(p.inputDB,-24.,36.));outputTarget=gain(std::clamp(p.outputDB,-36.,24.));
        m=modes[(size_t)p.ratio];
        if(p.british) {m.ratio=20;m.knee=1;m.threshold=-25;}
        // Feedback loop gain is R-1; scaling the controller time constant by R
        // preserves the small-signal closed-loop time constant and stability.
        attack=1-std::exp(-1/(fs*p.attackMS*.001*m.ratio));
        release=1-std::exp(-1/(fs*p.releaseMS*.001*m.ratio));
        memoryAlpha=1-std::exp(-1/(fs*1.5));
    }
    std::array<float,2> tick(float left,float right,int channels=2) {
        in+=smooth*(inputTarget-in);out+=smooth*(outputTarget-out);
        hpMix+=smooth*((p.audioHP?1.:0.)-hpMix);
        scMix+=smooth*((p.detectorHP?1.:0.)-scMix);
        eqMix+=smooth*((p.emphasis?1.:0.)-eqMix);
        c2+=smooth*((p.colour==1?1.:0.)-c2);c3+=smooth*((p.colour==2?1.:0.)-c3);
        std::array<double,2> x{{finite(left)*in,finite(right)*in}}, targets{};
        const int n=std::clamp(channels,1,2);
        for(int c=0;c<n;++c) {
            auto& s=ch[(size_t)c];
            // Manufacturer block diagram: detector taps AFTER distortion and
            // BEFORE audio HP. One oversampled-sample delay closes the causal loop.
            double feedback=s.feedback;
            const double e=s.eq.tick(feedback);feedback+=eqMix*(e-feedback);
            const double h=s.sc.tick(feedback);feedback+=scMix*(h-feedback);
            const double over=db(feedback)-m.threshold;
            double rect=std::max(0.,over);
            if(m.knee>0) {
                if(over<=-m.knee*.5) rect=0;
                else if(over<m.knee*.5) rect=(over+m.knee*.5)*(over+m.knee*.5)/(2*m.knee);
            }
            targets[(size_t)c]=(m.ratio-1)*rect;
        }
        if(n==2 && p.linked) {
            targets[0]=targets[1]=std::max(targets[0],targets[1]);
            // Re-linking must restore identical channel gain immediately.
            ch[0].gr=ch[1].gr=std::max(ch[0].gr,ch[1].gr);
            ch[0].memory=ch[1].memory=std::max(ch[0].memory,ch[1].memory);
        }
        std::array<float,2> result{};
        reduction=0;peakIn=std::max(std::abs(x[0]),n==2?std::abs(x[1]):0.);
        for(int c=0;c<n;++c) {
            auto& s=ch[(size_t)c]; const double target=std::min(120.,targets[(size_t)c]);
            s.memory+=memoryAlpha*(s.gr-s.memory);
            double rel=release;
            if(p.ratio==5 && !p.british) rel/=1+std::min(5.,s.memory*.2); // optical-style memory
            if(p.ratio==7 || p.british) rel/=1+s.gr*.045; // curved recovery
            s.gr+=(target>s.gr?attack:rel)*(target-s.gr);
            s.gr=std::clamp(s.gr,0.,90.);
            double v=x[(size_t)c]*gain(-s.gr);
            // Adjustable asymmetry generates even harmonics; symmetric tanh
            // generates odd harmonics. Coefficients are openly estimated.
            const double drive=1.4,bias=.23;
            const double even=(std::tanh(drive*v+bias)-std::tanh(bias))/(drive*(1-std::pow(std::tanh(bias),2)));
            const double odd=std::tanh(2.2*v)/2.2;
            v+=c2*(even-v)+c3*(odd-v);
            // DC blocker remains warm in clean mode; mix avoids colouring clean.
            const double dc=s.dc.tick(v);v+=(c2+c3)*(dc-v);
            s.feedback=v;
            const double h=s.hp.tick(v);v+=hpMix*(h-v);
            result[(size_t)c]=(float)(v*out);
            reduction=std::max(reduction,s.gr);
        }
        peakOut=std::max(std::abs((double)result[0]),std::abs((double)result[1]));
        return result;
    }
    double reduction=0,peakIn=0,peakOut=0;
private:
    static double finite(double x) {return std::isfinite(x)?std::clamp(x,-64.,64.):0.;}
    struct Channel { Biquad hp,sc,eq,dc; double gr=0,memory=0,feedback=0; };
    std::array<Channel,2> ch;
    Parameters p; Mode m=modes[4];
    double fs=48000,smooth=.001,in=1,out=1,inputTarget=1,outputTarget=1,
        hpMix=0,scMix=0,eqMix=0,c2=0,c3=0,attack=.001,release=.00001,memoryAlpha=.00001;
};
}
