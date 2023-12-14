#include "internal.hpp"



namespace AWC {

    static AWCData __global_instance;


    AWCData* getInstance() {
        return &__global_instance;
    }

    AWCData::WinContext& getActiveContext() {
        ifcrash_debug(!__global_instance.init || __global_instance.activeContext == DEFAULT32);

        return __global_instance.contexts[ 
            __global_instance.activeContext 
        ];
}


} // namespace AWC