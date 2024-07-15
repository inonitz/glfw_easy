#include "macro.hpp"
#include "instance.hpp"
#include "state.hpp"


namespace AWC {
    bool __awc_lib_initialized() {
        return AWC_LIB_GET_BITS(__get_instance()->flags, 
            AWC_LIB_INIT_MASK, AWC_LIB_INIT_SHIFT
        );
    }
    
    bool __awc_lib_atleast_one_existing_context() {
        return AWC_LIB_GET_BITS(__get_instance()->flags, 
            AWC_LIB_ATLEAST_ONE_CONTEXT_MASK, AWC_LIB_ATLEAST_ONE_CONTEXT_SHIFT
        );
    }
    
    u8 __awc_lib_active_context() {
        return AWC_LIB_GET_BITS(__get_instance()->flags, 
            AWC_LIB_ACTIVE_CONTEXT_MASK, AWC_LIB_ACTIVE_CONTEXT_SHIFT
        );
    }
    
    u8 __awc_lib_context_count() {
        return AWC_LIB_GET_BITS(__get_instance()->flags, 
            AWC_LIB_CONTEXT_COUNT_MASK, AWC_LIB_CONTEXT_COUNT_SHIFT
        );
    }


} // namespace AWC