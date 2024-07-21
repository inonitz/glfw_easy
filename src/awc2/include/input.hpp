#ifndef __AWC2_INPUT_INTERFACE_DEFINITION_HEADER__
#define __AWC2_INPUT_INTERFACE_DEFINITION_HEADER__
#include "input_types.hpp"


namespace AWC2::Input {
    bool isKeyPressed (keyCode key);
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
} // namespace AWC2::Input


#endif