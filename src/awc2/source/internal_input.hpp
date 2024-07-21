#ifndef __AWC2_INTERNAL_INPUT_CONTEXT_DEFINITION_HEADER__
#define __AWC2_INTERNAL_INPUT_CONTEXT_DEFINITION_HEADER__
#include "awc2/include/input_types.hpp"


namespace AWC2::internal {


struct alignsz(8) InputState
{
    u8 keyboardKeys[(u8)Input::keyCode::KEY_MAX + 1] = {0}; /* keyCode enum types are also used to index into the array */
    u8 mouseButtons[__scast(u8, Input::mouseButton::MAX) + 1] = {0};
    u8 mouseMovedFlag[2]  = {0};
    u8 scrollMovedFlag[2] = {0};
    Input::cursorPosition previousFramePos;
    Input::cursorPosition currentFramePos;
    Input::cursorPosition previousFrameScroll;
    Input::cursorPosition currentFrameScroll;
};


u16                toGLFWKeyCode(Input::keyCode kc);
Input::keyCode     toKeyCode(u16 glfw);
const char*        keyCodeToString(Input::keyCode kc);
u16                toGLFWMouseButton(Input::mouseButton button);
Input::mouseButton toMouseButton(u16 glfw);


} // namespace AWC2

#endif