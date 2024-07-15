#ifndef __UTIL_ALIGNED_MEMORY_ALLOCATOR__
#define __UTIL_ALIGNED_MEMORY_ALLOCATOR__
#include "types.hpp"


namespace util {


template<u16 alignment> void* aligned_malloc(u64 bytes);
                        void  aligned_free(void* ptr);


} // namespace util


#endif