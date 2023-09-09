#ifndef __AWC_PUBLIC_STRUCTURE_HEADER__
#define __AWC_PUBLIC_STRUCTURE_HEADER__
#include "util/base.hpp"
#include <vector>
#include "input.hpp"
#include "event.hpp"
#include "window.hpp"
#include "util/allocator.hpp"


namespace AWC {


struct AWCData
{
    typedef struct __window_input_pair {
        WindowContext*        win;
        Input::InputUnit*     unit;
        Event::callbackTable* callbacks;
    } WinContext;

    struct {
        StaticPoolAllocator<Input::InputUnit>     inputs;
        StaticPoolAllocator<WindowContext>        windows;
        StaticPoolAllocator<Event::callbackTable> handler_tables;
    } poolAlloc;
    std::vector<WinContext> contexts;
    u32                     activeContext;
};


AWCData* getInstance();


}

#endif