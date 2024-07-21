#include "awc2/include/input.hpp"
#include "instance.hpp"


namespace detail {

struct InputManager {

};


} // namespace detail



namespace AWC2::Input { /* Will work per-active-context */
    bool isKeyPressed (keyCode key) {
        
    }
    bool isKeyReleased(keyCode key);
    bool isKeyRepeated(keyCode key);
    bool isMouseButtonPressed (mouseButton but);
    bool isMouseButtonReleased(mouseButton but);
    bool isMouseMoving();
    bool isMouseScrollMoving();
    cursorPosition getMouseScrollOffset();
    cursorPosition getMousePosition();
    cursorPosition getMousePositionDelta();
    void setCursorMode(u8 mode);
} // namespace Input