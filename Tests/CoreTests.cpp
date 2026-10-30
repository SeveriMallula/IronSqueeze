#include "../Source/CompressorCore.h"
#include <iostream>
#include <random>
#include <stdexcept>
#include <limits>
using namespace iron;
void check(bool ok,const char* what){if(!ok)throw std::runtime_error(what);}
double settle(Core& c,double level,double fs,double seconds=3){
    float y=0;for(int i=0;i<(int)(seconds*fs);++i)y=c.tick((float)gain(level),(float)gain(level))[0];return db(y);
}
double rmsTone(Parameters p,double frequency,double fs=48000){
    Core c;c.prepare(fs);c.set(p);double sum=0;int n=0;
    for(int i=0;i<(int)(fs*3);++i){float x=(float)(.5*std::sin(2*pi*frequency*i/fs));auto y=c.tick(x,x);if(i>(int)(fs*2)){sum+=y[0]*y[0];++n;}}
    return std::sqrt(sum/n);
}
int main(){try{
    for(double fs:{44100.,48000.,96000.,192000.}){
        Core c;c.prepare(fs);Parameters p;p.ratio=0;c.set(p);
        for(int i=0;i<5000;++i){const float x=(float)(.5*std::sin(.13*i));check(std::abs(c.tick(x,x)[0]-x)<2e-7,"clean unity");}
        check(c.reduction<1e-9,"1:1 reduction");
    }
    std::cout<<"PASS clean 1:1 unity at 44.1/48/96/192 kHz\n";
    for(int mode=1;mode<8;++mode){
        Core c;c.prepare(48000);Parameters p;p.ratio=mode;p.attackMS=2;p.releaseMS=50;c.set(p);
        const double low=settle(c,6,48000),high=settle(c,12,48000);
        const double slope=(high-low)/6.;
        std::cout<<"Ratio "<<modes[(size_t)mode].ratio<<" slope "<<slope<<"\n";
        check(std::abs(slope-1/modes[(size_t)mode].ratio)<.012,"feedback static ratio slope");
    }
    std::cout<<"PASS ratio slopes using settled DC, above knee\n";
    {
        Core c;c.prepare(48000);Parameters p;p.ratio=4;c.set(p);
        double maxError=0;
        for(int i=0;i<96000;++i){float x=(float)(.7*std::sin(2*pi*400*i/48000));auto y=c.tick(x,x*.1f);maxError=std::max(maxError,std::abs((double)y[1]-.1*y[0]));}
        check(maxError<1e-6,"linked stereo balance");
        p.linked=false;c.set(p);auto y=c.tick(.8f,.08f);check(std::isfinite(y[0]),"unlink stability");
    }
    std::cout<<"PASS linked stereo balance\n";
    {
        Parameters p;p.ratio=0;double a=rmsTone(p,30);p.audioHP=true;double b=rmsTone(p,30);
        check(db(b/a)<-15,"audio HP low attenuation");
        p.audioHP=false;p.ratio=4;double noHP=rmsTone(p,30);p.detectorHP=true;double withHP=rmsTone(p,30);
        check(withHP>noHP*1.5,"detector HP reduces bass pumping");
        p.detectorHP=false;double noBell=rmsTone(p,6000);p.emphasis=true;double bell=rmsTone(p,6000);
        check(bell<noBell*.85,"detector emphasis increases compression");
    }
    std::cout<<"PASS audio and detector filter behaviour\n";
    {
        Core c;c.prepare(48000);Parameters p;p.ratio=4;p.attackMS=30;c.set(p);
        settle(c,-70,48000,.1);c.tick(.8f,.8f);double first=c.reduction;
        for(int i=0;i<48000;++i)c.tick(.8f,.8f);
        double held=c.reduction;
        check(held>first+5,"attack envelope");
        for(int i=0;i<240000;++i)c.tick(0,0);
        check(c.reduction<held*.01,"release envelope");
    }
    std::cout<<"PASS attack/release recovery\n";
    {
        const int N=48000;std::array<std::array<double,3>,3> harmonic{};
        for(int mode=0;mode<3;++mode){Core c;c.prepare(48000);Parameters p;p.ratio=0;p.colour=mode;c.set(p);
            for(int i=0;i<2*N;++i){double phase=2*pi*1000*i/48000;auto y=c.tick((float)(.6*std::sin(phase)),0,1);
                if(i>=N)for(int h=1;h<=3;++h)harmonic[(size_t)mode][(size_t)(h-1)]+=y[0]*std::sin(h*phase);}
        }
        // H2 has a quadrature phase, so explicitly measure cosine as well.
        Core c;c.prepare(48000);Parameters p;p.ratio=0;p.colour=1;c.set(p);double even=0;
        for(int i=0;i<2*N;++i){double phase=2*pi*1000*i/48000;auto y=c.tick((float)(.6*std::sin(phase)),0,1);if(i>=N)even+=y[0]*std::cos(2*phase);}
        check(std::abs(even)>20,"H2 produces second harmonic");
        check(std::abs(harmonic[2][2])>20,"H3 produces third harmonic");
    }
    std::cout<<"PASS even and odd harmonics\n";
    {
        const auto meanGR=[](int colour,bool audioHP){Core c;c.prepare(48000);Parameters p;p.inputDB=18;p.colour=colour;p.audioHP=audioHP;c.set(p);double sum=0;
            for(int i=0;i<144000;++i){float x=(float)(.5*std::sin(2*pi*1000*i/48000));c.tick(x,x);if(i>=96000)sum+=c.reduction;}return sum/48000.;};
        const double clean=meanGR(0,false),coloured=meanGR(2,false),filtered=meanGR(2,true);
        check(coloured<clean-.02,"post-colour detector responds to saturation");
        check(std::abs(coloured-filtered)<1e-9,"audio HP outside feedback loop");
        std::cout<<"PASS post-colour detector tap and audio HP outside loop\n";
    }
    {
        Core c;c.prepare(192000);Parameters p;std::mt19937 rng(41);std::uniform_real_distribution<float> noise(-4,4);
        for(int i=0;i<500000;++i){if(i%1000==0){p.ratio=(i/1000)%8;p.colour=(i/8000)%3;p.inputDB=(i/24000)%60-24;p.outputDB=-6;p.attackMS=.05;p.releaseMS=50;p.british=(i/1000)%2;p.linked=(i/2000)%2;p.audioHP=(i/4000)%2;p.detectorHP=(i/8000)%2;p.emphasis=(i/16000)%2;c.set(p);}
            auto y=c.tick(noise(rng),noise(rng));check(std::isfinite(y[0])&&std::isfinite(y[1]),"automation finite output");}
        auto y=c.tick(std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity());check(std::isfinite(y[0])&&std::isfinite(y[1]),"nonfinite containment");
    }
    std::cout<<"PASS extreme automation and nonfinite input\nALL CORE TESTS PASSED\n";
    return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}}
