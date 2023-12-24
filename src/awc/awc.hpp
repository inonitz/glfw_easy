#ifndef __AWC_CONTEXT_HEADER__
#define __AWC_CONTEXT_HEADER__
#include "inputdef.hpp"
#include "windowdef.hpp"
#include "event.hpp"


struct GladGLContext;


namespace AWC {


void init();
void destroy();
void begin_frame(); /* A Context MUST be bound before calling the function, because it acts on the active context.  */
void end_frame();   /* A Context MUST be bound before calling the function, because it acts on the active context.  */


namespace Context {
    /* 
        Creates a new context, which includes:
            Input Unit
            GLFW Window
            Event Handler Table
            OpenGL Context
            ImGui Context
        * if return_value == 0:
            Failed to create new Context, error will be printed.
        * else:
            Context Created Successfully -> u8 returned is the ID of the context.
    */
    u8   allocate();
    bool init(
        AWC::WindowOptions        const& options,
        AWC::WindowDescriptor     const& desc,
        AWC::Event::callbackTable const& override = {}
    );
    void setActive(u8 id);
    
    
    /*
        is the underlying GLFW window still open?
    */
    bool childWindowActive(u8 id);
    GladGLContext* childGL();

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