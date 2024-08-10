#include "include/input.hpp"
#include "internal_input.hpp"
#include "internal_instance.hpp"
#include "internal_state.hpp"
#include <GLFW/glfw3.h>
#include "source/internal_instance.hpp"


namespace AWC2::Input { /* Will work per-active-context */


#define current_input_context() \
    internal::__awc2_lib_get_active_context().m_io \

#define in_ctx() current_input_context()


    bool isKeyPressed (keyCode key) { 
        return __scast(Input::inputState, 
                in_ctx().keyboardKeys[__scast(u8, key)] 
            ) == inputState::PRESS;
    }
    bool isKeyReleased(keyCode key) { 
        return __scast(Input::inputState, 
                in_ctx().keyboardKeys[__scast(u8, key)] 
            ) == inputState::RELEASE;
    }
    bool isKeyRepeated(keyCode key) { 
        return __scast(Input::inputState, 
                in_ctx().keyboardKeys[__scast(u8, key)] 
            ) == inputState::REPEAT;
    }
    bool isMouseButtonPressed (mouseButton but) {
        return __scast(inputState, 
                in_ctx().mouseButtons[__scast(u8, but)]
            ) == inputState::PRESS;
    }
    bool isMouseButtonReleased(mouseButton but) {
        return __scast(inputState, 
                in_ctx().mouseButtons[__scast(u8, but)]
            ) == inputState::RELEASE;
    }
    

    bool isMouseMoving()       { return in_ctx().mouseMovedFlag[1];  }
    bool isMouseScrollMoving() { return in_ctx().scrollMovedFlag[1]; }
    cursorPosition getMouseScrollOffset() { return in_ctx().currentFrameScroll; }
    cursorPosition getMousePosition()     { return in_ctx().currentFramePos;    }
    cursorPosition getMousePositionDelta() {
        auto& mouseState = in_ctx();
        return cursorPosition{{{ /* Y axis is flipped on GLFW (X_axis = right, Y_axis = down) */
            (mouseState.currentFramePos.x  - mouseState.previousFramePos.x),
            (mouseState.previousFramePos.y - mouseState.currentFramePos.y )
        }}};
    }


    void setCursorMode(Input::cursorMode mode) {
        static constexpr u32 modeToValue[4] = {
            GLFW_CURSOR_HIDDEN,
            GLFW_CURSOR_NORMAL,
            GLFW_CURSOR_DISABLED,
            GLFW_CURSOR_CAPTURED
        };
        glfwSetInputMode(internal::__awc2_lib_get_active_context().m_window.underlying_handle(),
            GLFW_CURSOR,
            modeToValue[ __scast(u8, mode) ]
        );
        return;
    }


} // namespace AWC2::Input