#ifndef __AWC_CONTEXT_HEADER__
#define __AWC_CONTEXT_HEADER__
#include "util/base.hpp"
#include "inputdef.hpp"


#define MAXIMUM_CONTEXTS 5


namespace AWC {


void init();
void destroy();
void begin_frame();
void end_frame();

/* 
    Creates a new context, which includes:
        Input Unit,
        GLFW Window,
        Event Handler Table
    * if return_value == 0:
        Failed to create new Context, error will be printed.
    * else:
        Context Created Successfully -> u8 returned is the ID of the context.
*/
u8   allocateContext();
void initContext();
void setActiveContext(u8 id);


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
}


}  // namespace AWC

#endif