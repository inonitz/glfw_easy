#include "instance.hpp"
#include "state.hpp"
#include "macro.hpp"
#include "util/ifcrash.hpp"
#include "util/macro.hpp"


namespace AWC {


static AWCData __global_instance;


AWCData* __get_instance() { return &__global_instance; }


AWCContext* __get_context(unsigned char id) {
    __release_unused auto condition = (id == 0) 
        || (id > __awc_lib_context_count())
        || !__awc_lib_initialized();

    ifcrashstr_debug(condition, "AWC::activeContext() => No Active Context Selected/Allocated!\n");
    return &__global_instance.contexts[ id - 1 ];
}

AWCContext* __active_context() {
    __release_unused auto condition = (__awc_lib_active_context() == 0) 
        || (__awc_lib_active_context() == 0)
        || !__awc_lib_initialized();

    ifcrashstr_debug(condition, "AWC::activeContext() => No Active Context Selected/Allocated!\n");
    return &__global_instance.contexts[ __awc_lib_active_context() - 1];
}


unsigned char __active_context_id() {
    return __awc_lib_active_context() - 1;
}


} // namespace AWC