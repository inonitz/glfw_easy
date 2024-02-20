#ifndef __AWC_CONTEXT_HEADER__
#define __AWC_CONTEXT_HEADER__
#include "inputdef.hpp"
#include "windowdef.hpp"
#include "event.hpp"


namespace AWC {


void init();
void destroy();
void __hot begin_frame(); /* A Context MUST be bound before calling the function, because it acts on the active context.  */
void __hot end_frame();   /* A Context MUST be bound before calling the function, because it acts on the active context.  */


namespace Context {
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
        AWC::WindowOptions        const& options,
        AWC::WindowDescriptor     const& desc,
        AWC::Event::callbackTable const& override = {}
    );
    void setActive(u8 id);
    bool windowActive(u8 id);
    std::array<u32, 2> windowSize(u8 id);
}


namespace Input { /* Will work per-active-context */
    void reset();
    bool isKeyPressed (keyCode key);
    bool isKeyReleased(keyCode key);
    bool isMouseButtonPressed (mouseButton but);
    bool isMouseButtonReleased(mouseButton but);
    void lockCursor();
    void unlockCursor();
    void setCursorMode(bool lock);
}


namespace Event {
    template<class Func> void overrideHandler(Func* handlerAddress);
    template<class Func, bool nullptrOrDefault> void resetHandler();
}


}  // namespace AWC

#endif