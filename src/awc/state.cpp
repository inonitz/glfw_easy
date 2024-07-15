#include "state.hpp"
#include "util/print.hpp"


void AWC::AWCData::print() const 
{
    u32 i = 0;
    util::print("AWCData::print() {\n");
    for(auto& ctxt : contexts)
    {
        util::printfmt("Window-Context Unit %u\n", i++);
        util::printfmt("  win       0x%p\n  unit      0x%p\n  library callback table 0x%p\n  user callback table 0x%p\n  imgui     0x%p\n",
            __scast(void*, ctxt.win), 
            __scast(void*, ctxt.unit), 
            __scast(void*, ctxt.callbacks),
            __scast(void*, ctxt.usercallbacks),
            __scast(void*, ctxt.imgui)
        );
    }
    util::print("}\n");
    return;
}