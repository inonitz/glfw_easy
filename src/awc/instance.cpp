#include "instance.hpp"



namespace AWC {

    static AWCData __global_instance;


    AWCData* getInstance() {
        return &__global_instance;
    }

} // namespace AWC