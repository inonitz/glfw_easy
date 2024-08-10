#include "internal_instance.hpp"
#include "internal_state.hpp"


namespace AWC2::internal {


static AWC2Data __library_local_data;


AWC2Data* __awc2_lib_get_instance()
{
    return &__library_local_data;
}

AWC2ContextData& __awc2_lib_get_context(unsigned char id) {
    return __library_local_data.poolmem[--id];
}

AWC2ContextData& __awc2_lib_get_active_context()
{
    return __awc2_lib_get_context(__library_local_data.activeid);
}

unsigned char __awc2_lib_get_active_context_id()
{
    return __library_local_data.activeid;
}


} // namespace AWC2::internal