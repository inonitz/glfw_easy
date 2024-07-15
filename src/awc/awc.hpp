#ifndef __AWC_LIBRAY_HEADER__
#define __AWC_LIBRAY_HEADER__
#include "inputdef.hpp"
#include "context.hpp"
#include <array>


namespace AWC {


void init();
void destroy();
void __hot begin_frame(); /* A Context MUST be bound before calling the function, because it acts on the active context.  */
void __hot end_frame();   /* A Context MUST be bound before calling the function, because it acts on the active context.  */


namespace Input { /* Will work per-active-context */
    void reset();
    bool isKeyPressed (keyCode key);
    bool isKeyReleased(keyCode key);
    bool isKeyRepeated(keyCode key);
    bool isMouseButtonPressed (mouseButton but);
    bool isMouseButtonReleased(mouseButton but);
    bool isMouseMoving();
    bool isMouseScrollMoving();
    std::array<f32, 2> getPreviousMousePosition();
    std::array<f32, 2> getMousePosition();
    std::array<f32, 2> getMouseScrollOffset();
    std::array<f32, 2> getMousePositionDelta();
    void unrestrictCursor();
    void unlockCursor();
    void restrictCursor();
    void hideCursor();
    void setCursorMode(u8 mode);
} // namespace Input


namespace Event {
    template<class Func> void setUserCallback(Func handlerAddress = nullptr);
    template<class Func> void overrideLibraryHandler(Func* handlerAddress);
    template<class Func> void resetLibraryHandler();
} // namespace Event


}  // namespace AWC

#endif