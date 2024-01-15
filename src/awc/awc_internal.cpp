#include "awc_internal.hpp"


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