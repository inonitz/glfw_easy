#ifndef __AWC_INTERNAL_STRUCTURE_HEADER__
#define __AWC_INTERNAL_STRUCTURE_HEADER__
#include "util/base.hpp"
#include <glad2/gl.h>
#include <vector>
#include "input.hpp"
#include "event.hpp"
#include "window.hpp"
#include "util/allocator.hpp"


namespace AWC {


struct AWCData
{
    typedef struct __window_input_pair {
        Input::InputUnit*     unit;
        Event::callbackTable* callbacks;
        WindowContext*        win;
        GladGLContext*        gl;
    } WinContext;

    struct {
        StaticPoolAllocator<Input::InputUnit>     inputs;
        StaticPoolAllocator<Event::callbackTable> handler_tables;
        StaticPoolAllocator<WindowContext>        windows;
        StaticPoolAllocator<GladGLContext>        glctxt;
    } poolAlloc;
    std::vector<WinContext> contexts;
    u32                     activeContext = DEFAULT32;
    bool                    __init_lib = false;
    bool                    __flags[3];
};


AWCData*             getInstance();
AWCData::WinContext& getActiveContext();


}

#endif // namespace AWC