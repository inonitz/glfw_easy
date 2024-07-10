#include "awc_internal.hpp"
#include "util/ifcrash.hpp"
#include <cstdio>


void AWC::AWCData::print() const {
    u32 i = 0;
    printf("AWCData::print() {\n");
    for(auto& ctxt : contexts)
    {
        printf("Window-Context Unit %u\n", i++);
        printf("  win       0x%p\n  unit      0x%p\n  library callback table 0x%p\n  user callback table 0x%p\n  imgui     0x%p\n",
            __scast(void*, ctxt.win), 
            __scast(void*, ctxt.unit), 
            __scast(void*, ctxt.callbacks),
            __scast(void*, ctxt.usercallbacks),
            __scast(void*, ctxt.imgui)
        );
    }
    printf("}\n");
}


namespace AWC {


static AWCData __global_instance;


AWCData* getInstance() {
    return &__global_instance;
}


AWCData::WinContext& activeContext()
{
    auto condition = (AWC_LIB_ACTIVE_CONTEXT() == 0);
    condition = condition || (AWC_LIB_CONTEXT_COUNT() == 0);
    condition = condition || !AWC_LIB_INITIALIZED();

    ifcrashdo_debug((  (AWC_LIB_ACTIVE_CONTEXT() == 0) 
        || (AWC_LIB_CONTEXT_COUNT() == 0) 
        || !AWC_LIB_INITIALIZED()  ), {
        fprintf(stderr, "AWC::activeContext() => No Active Context Selected/Allocated!\n");
    });
    return __global_instance.contexts[ AWC_LIB_ACTIVE_CONTEXT() - 1];
}


} // namespace AWC