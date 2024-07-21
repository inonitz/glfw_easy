#ifndef __AWC2_INTERNAL_STATE_DEFINITION_HEADER__
#define __AWC2_INTERNAL_STATE_DEFINITION_HEADER__
#include "awc2/source/internal_event.hpp"
#include "awc2/source/internal_window.hpp"
#include "awc2/source/internal_input.hpp"


namespace AWC2::internal {


struct AWC2ContextData
{
    Window            window;
    userCallbackTable event_table;
    InputState        io;
    void*             imgui;


    void create();
    void destroy();
};


struct AWC2Data
{
    
};


} // namespace AWC2::internal


#endif