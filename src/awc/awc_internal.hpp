#ifndef __AWC_PUBLIC_STRUCTURE_HEADER__
#define __AWC_PUBLIC_STRUCTURE_HEADER__
#include "util/base.hpp"
#include "util/allocator.hpp"
#include <array>
#include <glad/gl.h>
#include <ImGui/imgui_internal.h>


namespace AWC {

namespace Input { class alignsz(64) InputUnit; }
namespace Event { struct callbackTable;        }
struct WindowContext;


#define AWC_LIB_CONTEXT_MAX              (0b111)
#define AWC_LIB_INIT_MASK                (0x1)
#define AWC_LIB_ATLEAST_ONE_CONTEXT_MASK (0b00000010)
#define AWC_LIB_ACTIVE_CONTEXT_MASK      (0b00011100)
#define AWC_LIB_CONTEXT_COUNT_MASK       (0b11100000)

#define AWC_LIB_INIT_SHIFT                (0)
#define AWC_LIB_ATLEAST_ONE_CONTEXT_SHIFT (1)
#define AWC_LIB_ACTIVE_CONTEXT_SHIFT      (2)
#define AWC_LIB_CONTEXT_COUNT_SHIFT       (5)

#define AWC_LIB_GET_BITS(num, bitmask, shift) ( ( (num) & (bitmask) ) >> (shift) )
#define AWC_LIB_RESET_BITS(num, bitmask) (num) &= ~bitmask
#define AWC_LIB_SET_BITS(num, bitmask) (num) |= bitmask
#define AWC_LIB_RESET_SET_BITS(num, bitmask, newBits) ( (num & ~(bitmask) ) | (newBits) );
#define AWC_LIB_MODIFY_VAR_BITS(num, bitmask, newBits) { num = ( num & ~(bitmask) ) | (newBits); }


struct AWCData
{
    typedef struct alignsz(64) __cache_aligned_gl_context {
        GladGLContext gl;
    } CachedGLContext;


    typedef struct __window_input_pair {
        WindowContext*        win;
        Input::InputUnit*     unit;
        Event::callbackTable* callbacks;
        GladGLContext*        opengl;
        ImGuiContext*         imgui;
    } WinContext;


    struct {
        template<typename T> 
        using SharedMemPool = StaticPoolAllocator<T, true>;
        
        using underlying_massive_memory = void*;
        using umm_pool = underlying_massive_memory;

        
        umm_pool                            global_shared;
#ifdef _DEBUG
        size_t                              global_size;
#endif
        SharedMemPool<Input::InputUnit>     inputs;
        SharedMemPool<WindowContext>        windows;
        SharedMemPool<Event::callbackTable> handler_tables;
        SharedMemPool<CachedGLContext>      gl;
    } poolAlloc;
    

    std::array<WinContext, 8> contexts;
    /* 
        flags definition:
        u8 initialized            : 1;
        u8 allocated_atleast_once : 1;
        u8 activeContext          : 3;
        u8 numberOfContexts       : 3;
    */
    u8                      flags;
    u8                      reserved[7];

    
    void print() {
        u32 i = 0;
        printf("AWCData::print() {\n");
        for(auto& ctxt : contexts)
        {
            printf("Window-Context Unit %u\n", i++);
            printf("  win       0x%p\n  unit      0x%p\n  callbacks 0x%p\n  opengl    0x%p\n  imgui     0x%p\n",
                __scast(void*, ctxt.win), 
                __scast(void*, ctxt.unit), 
                __scast(void*, ctxt.callbacks), 
                __scast(void*, ctxt.opengl), 
                __scast(void*, ctxt.imgui)
            );
        }
        printf("}\n");
    }
};


AWCData*             getInstance();
AWCData::WinContext& activeContext();


#define AWC_LIB_INITIALIZED()          AWC_LIB_GET_BITS(getInstance()->flags, AWC_LIB_INIT_MASK               , AWC_LIB_INIT_SHIFT               )
#define AWC_LIB_CONTEXT_ATLEAST_ONCE() AWC_LIB_GET_BITS(getInstance()->flags, AWC_LIB_ATLEAST_ONE_CONTEXT_MASK, AWC_LIB_ATLEAST_ONE_CONTEXT_SHIFT)
#define AWC_LIB_ACTIVE_CONTEXT()       AWC_LIB_GET_BITS(getInstance()->flags, AWC_LIB_ACTIVE_CONTEXT_MASK     , AWC_LIB_ACTIVE_CONTEXT_SHIFT     )
#define AWC_LIB_CONTEXT_COUNT()        AWC_LIB_GET_BITS(getInstance()->flags, AWC_LIB_CONTEXT_COUNT_MASK      , AWC_LIB_CONTEXT_COUNT_SHIFT      )

} // namespace AWC

#endif