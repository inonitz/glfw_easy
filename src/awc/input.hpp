#ifndef __AWC_INPUT_STRUCT_HEADER__
#define __AWC_INPUT_STRUCT_HEADER__
#include "inputdef.hpp"
#include "util/base.hpp"
#include <array>


namespace AWC::Input {


class InputUnit
{
public:
    using MousePrecisionType = f32;
    using screenPos = std::array<MousePrecisionType, 2>;

    void reset() {
        // memset(mouseState.movement, 0x00, sizeof(mouseState.movement));
        memset(keyboardState.keys,  0x00, sizeof(keyboardState.keys ));
        mouseState.mouseMovement[0] = mouseState.mouseMovement[1];
        mouseState.mouseMovement[1] = false;
        return;
    }


    template<typename T> 
    std::array<T, 2> getCurrentFrameCursorPos() const {
        return {
            __scast(T, mouseState.currentFramePos[0]),
            __scast(T, mouseState.currentFramePos[1])
        };
    }
    template<typename T> 
    std::array<T, 2> getPreviousFrameCursorPos() const {
        return {
            __scast(T, mouseState.previousFramePos[0]),
            __scast(T, mouseState.previousFramePos[1])
        };
    }
    template<typename T> 
    std::array<T, 2> getCursorDelta() const { 
    return { /* Y axis is flipped on GLFW (X_axis = right, Y_axis = down) */
        __scast(T, (mouseState.currentFramePos[0]  - mouseState.previousFramePos[0]) ),
        __scast(T, (mouseState.previousFramePos[1] - mouseState.currentFramePos[1] ) )
        };
    }

    template<typename T> 
    std::array<T, 2> getCurrentFrameScrollOffset() const {
        return {
            __scast(T, mouseState.currentFrameScroll[0]),
            __scast(T, mouseState.currentFrameScroll[1])
        };
    }
    template<typename T> 
    std::array<T, 2> getPreviousFrameScrollOffset() const {
        return {
            __scast(T, mouseState.previousFrameScroll[0]),
            __scast(T, mouseState.previousFrameScroll[1])
        };
    }


    __force_inline void updateMousePosition(screenPos const& newPosition) {
        mouseState.previousFramePos = mouseState.currentFramePos;
        mouseState.currentFramePos = newPosition;
        mouseState.mouseMovement[1] = true;
        return;
    }
    __force_inline void updateScrollOffset(screenPos const& newOffset) {
        mouseState.previousFrameScroll = mouseState.currentFrameScroll;
        mouseState.currentFrameScroll = newOffset;
        return;
    }


    __force_inline inputState getKeyState (keyCode key) const {
        return __scast( inputState, keyboardState.keys[__scast(u8, key)] );
    }
    __force_inline inputState getMouseButtonState(mouseButton key) const { 
        return __scast( inputState, mouseState.buttons[__scast(u8, key)] );
    }
    __force_inline u8 getMouseMovementState() const { 
        return mouseState.mouseMovement[1];
    }
    __force_inline u8 getScrollMovementState() const { 
        return mouseState.scrollMovement[1];
    }

    __force_inline void setKeyState(keyCode key, u8 state) {
        keyboardState.keys[__scast(u8, key)] = state;
        return;
    }
    __force_inline void setMouseButtonState(mouseButton key, u8 state) {
        mouseState.buttons[__scast(u8, key)] = state;
        return;
    }


private:
    struct KeyboardState {
        u8 keys[(u8)keyCode::KEY_MAX + 1] = {0}; /* keyCode enum types are also used to index into the array */
    };

    struct MouseButtonState {
        std::array<MousePrecisionType, 2> previousFramePos;
        std::array<MousePrecisionType, 2> currentFramePos;
        std::array<MousePrecisionType, 2> previousFrameScroll;
        std::array<MousePrecisionType, 2> currentFrameScroll;
        u8 buttons[static_cast<u8>(mouseButton::MAX) + 1] = {0};
        u8 mouseMovement[2] = {0};
        u8 scrollMovement[2] = {0};
    };


private:
    MouseButtonState mouseState;
    KeyboardState    keyboardState;
};


template std::array<f32, 2> InputUnit::getCurrentFrameCursorPos<f32>() const;
template std::array<f64, 2> InputUnit::getCurrentFrameCursorPos<f64>() const;
template std::array<u32, 2> InputUnit::getCurrentFrameCursorPos<u32>() const;
template std::array<f32, 2> InputUnit::getPreviousFrameCursorPos<f32>() const;
template std::array<f64, 2> InputUnit::getPreviousFrameCursorPos<f64>() const;
template std::array<u32, 2> InputUnit::getPreviousFrameCursorPos<u32>() const;
template std::array<f32, 2> InputUnit::getCurrentFrameScrollOffset<f32>() const;
template std::array<f64, 2> InputUnit::getCurrentFrameScrollOffset<f64>() const;
template std::array<u32, 2> InputUnit::getCurrentFrameScrollOffset<u32>() const;
template std::array<f32, 2> InputUnit::getPreviousFrameScrollOffset<f32>() const;
template std::array<f64, 2> InputUnit::getPreviousFrameScrollOffset<f64>() const;
template std::array<u32, 2> InputUnit::getPreviousFrameScrollOffset<u32>() const;
template std::array<f32, 2> InputUnit::getCursorDelta<f32>() const;
template std::array<f64, 2> InputUnit::getCursorDelta<f64>() const;
template std::array<u32, 2> InputUnit::getCursorDelta<u32>() const;

}  // namespace AWC::Input


#endif