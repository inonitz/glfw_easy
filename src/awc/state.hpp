#ifndef __AWC_INTERNAL_STRUCTURE_DEFINITION_HEADER__
#define __AWC_INTERNAL_STRUCTURE_DEFINITION_HEADER__
#include "contextdef.hpp"
#include "util/pool.hpp"


namespace AWC {


class AWCData
{
private:
    typedef struct __single_block_of_shared_memory_multiple_pool_allocators 
    {
        template<u32 sizeofObjectInBytes> 
        using SharedBufferPool  = Pool<sizeofObjectInBytes, true>;

        using InputUnitPool     = SharedBufferPool<sizeof(AWC::Input::InputUnit)>;
        using WindowPool        = SharedBufferPool<sizeof(AWC::WindowContext)>;
        using libFuncTablePool  = SharedBufferPool<sizeof(AWC::Event::callbackTable)>;
        using userFuncTablePool = SharedBufferPool<sizeof(AWC::Event::userCallbackTable)>;
        using underlying_massive_memory = void*;


        underlying_massive_memory global_shared;
        debug_declaration_nobr (
            u64 global_size;
        )
        InputUnitPool     inputs;
        WindowPool        windows;
        libFuncTablePool  handler_tables;
        userFuncTablePool userhandler_tables;
    } SharedBufferPools;

public:
    SharedBufferPools         mempool;
    std::array<AWCContext, 8> contexts;
    /* 
        flags definition:
        u8 initialized            : 1;
        u8 allocated_atleast_once : 1;
        u8 activeContext          : 3;
        u8 numberOfContexts       : 3;
    */
    u8                      flags;
    u8                      reserved[7];

    void print() const;
};


} // namespace AWC


#endif