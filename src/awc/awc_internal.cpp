#include "awc_internal.hpp"


namespace AWC {

    static AWCData __global_instance;


    AWCData* getInstance() {
        return &__global_instance;
    }


    AWCData::WinContext& activeContext()
    {
        ifcrashdo_debug(
            AWC_LIB_ACTIVE_CONTEXT() == 0 
            || 
            AWC_LIB_CONTEXT_COUNT() == 0, 
        {
            fprintf(stderr, "AWC::activeContext() => No Active Context Selected/Allocated!\n");
        });
        return __global_instance.contexts[ AWC_LIB_ACTIVE_CONTEXT() ];
    }

} // namespace AWC