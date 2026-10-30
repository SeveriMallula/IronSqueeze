#include <pluginterfaces/base/ipluginbase.h>
#include <dlfcn.h>
#include <iostream>
#include <cstring>
int main(int argc,char** argv){
    if(argc!=2)return 2;
    void* library=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);
    if(!library){std::cerr<<dlerror()<<'\n';return 1;}
    auto entry=reinterpret_cast<bool(*)(void*)>(dlsym(library,"ModuleEntry"));
    auto exitModule=reinterpret_cast<bool(*)()>(dlsym(library,"ModuleExit"));
    auto get=reinterpret_cast<Steinberg::IPluginFactory*(*)()>(dlsym(library,"GetPluginFactory"));
    if(!entry||!exitModule||!get||!entry(library)){std::cerr<<"VST3 module entry failed\n";return 1;}
    auto* factory=get();if(!factory){std::cerr<<"No factory\n";return 1;}
    bool found=false;
    for(Steinberg::int32 i=0;i<factory->countClasses();++i){Steinberg::PClassInfo info{};
        if(factory->getClassInfo(i,&info)==Steinberg::kResultOk){std::cout<<info.name<<" / "<<info.category<<'\n';if(std::strstr(info.name,"Iron Squeeze")&&std::strcmp(info.category,"Audio Module Class")==0)found=true;}}
    factory->release();exitModule();dlclose(library);
    if(!found)return 1;
    std::cout<<"PASS compiled VST3 load, module entry, factory and audio class\n";return 0;
}
