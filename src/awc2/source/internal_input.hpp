#ifndef __AWC2_INTERNAL_INPUT_CONTEXT_DEFINITION_HEADER__
#define __AWC2_INTERNAL_INPUT_CONTEXT_DEFINITION_HEADER__
#include "include/input_types.hpp"


namespace AWC2::internal {


struct alignsz(8) InputState
{
    u8 keyboardKeys[__scast(u8, Input::keyCode::ENUM_MAX)     + 1] = {0}; /* keyCode enum types are also used to index into the array */
    u8 mouseButtons[__scast(u8, Input::mouseButton::ENUM_MAX) + 1] = {0};
    u8 mouseMovedFlag[2]  = {0};
    u8 scrollMovedFlag[2] = {0};
    Input::cursorPosition previousFramePos{};
    Input::cursorPosition currentFramePos{};
    Input::cursorPosition previousFrameScroll{};
    Input::cursorPosition currentFrameScroll{};

    void              reset();
    Input::inputState getKeyState(Input::keyCode key);
    void              setKeyState(Input::keyCode key, u8 state);
    void              updateMousePosition(Input::cursorPosition const& update);
    void              updateScrollOffset (Input::cursorPosition const& update);
    Input::inputState getMouseButtonState(Input::mouseButton button);
    void              setMouseButtonState(Input::mouseButton button, u8 state);
};


u16                toGLFWKeyCode(Input::keyCode kc);
Input::keyCode     toKeyCode(u16 glfw);
const char*        keyCodeToString(Input::keyCode kc);
u16                toGLFWMouseButton(Input::mouseButton button);
Input::mouseButton toMouseButton(u16 glfw);


} // namespace AWC2


#endif