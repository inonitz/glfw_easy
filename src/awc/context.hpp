#ifndef __AWC_CONTEXT_HEADER__
#define __AWC_CONTEXT_HEADER__
#include "windowdef.hpp"
#include "event.hpp"
#include <array>


namespace AWC::Context {
    /* 
        Creates a new context, which includes:
            * Input Unit, 
            * GLFW Window, 
            * Event Handler Table, 
            * OpenGL Context, 
            * ImGui Context, 
        * u8 return_value -> ID of the context.
        * if return_value == 0 => Context allocation failed + Error msg
    */
    u8   allocate();
    bool init(
        u8                               context_id,
        AWC::WindowOptions        const& options,
        AWC::WindowDescriptor     const& desc,
        AWC::Event::callbackTable const& override = {}
    );
    void setActive(u8 id);
    
    
    void begin();
    void end();
    

    bool isActive(u8 id);
    bool shouldClose(u8 id);
    template<typename T> std::array<T, 2> windowSize(u8 id);
} // namespace AWC::Context


#endif