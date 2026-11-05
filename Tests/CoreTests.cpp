#include "../Source/ImagerCore.h"
#include <iostream>
#include <random>
#include <stdexcept>
using namespace soundimager;
void check(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
int main(){
 try {
 std::mt19937 rng(42);std::uniform_real_distribution<float> d(-1,1);
 Core c;c.prepare(48000);
 for(int i=0;i<10000;++i){float l=d(rng),r=d(rng),x=l,y=r;c.process(x,y);check(std::abs(x-l)<1e-6f&&std::abs(y-r)<1e-6f,"unity transparency");}
 Settings s;s.width=0;c.prepare(48000,s);for(int i=0;i<1000;++i){float l=d(rng),r=d(rng);c.process(l,r);check(std::abs(l-r)<1e-6f,"mono fold");}
 s={};s.width=3;c.prepare(48000,s);for(int i=0;i<1000;++i){float l=d(rng),r=d(rng),sum=l+r;c.process(l,r);check(std::abs(l+r-sum)<1e-6f,"width preserves mono sum");}
 s={};s.rotation=45;c.prepare(48000,s);for(int i=0;i<1000;++i){float l=d(rng),r=d(rng),e=l*l+r*r;c.process(l,r);check(std::abs(l*l+r*r-e)<1e-6f,"rotation energy");}
 s={};s.swap=1;c.prepare(48000,s);float l=.4f,r=-.2f;c.process(l,r);check(std::abs(l+.2f)<1e-6f&&std::abs(r-.4f)<1e-6f,"channel swap");
 s={};s.bass=200;s.width=3;c.prepare(48000,s);double energy=0;for(int i=0;i<48000;++i){l=float(std::sin(6.283185307179586*30*i/48000));r=-l;c.process(l,r);if(i>24000)energy+=l*l;}check(energy/23999<0.11,"bass side attenuation");
 for(double sr:{44100.,48000.,96000.,192000.}){c.prepare(sr);s={3,45,12,1,12,500,0,1,0};c.set(s);for(int i=0;i<100000;++i){l=d(rng);r=d(rng);c.process(l,r);check(std::isfinite(l)&&std::isfinite(r),"finite extreme automation");}}
 s={};s.width=3;s.bypass=1;c.prepare(48000,s);l=.234f;r=-.786f;c.process(l,r);check(std::abs(l-.234f)<1e-6f&&std::abs(r+.786f)<1e-6f,"bypass");
 std::cout<<"PASS: unity, mono, mono-sum, rotation, swap, bass, automation, bypass\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
