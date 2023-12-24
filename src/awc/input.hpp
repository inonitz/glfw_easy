#ifndef __AWC_INPUT_STRUCT_HEADER__
#define __AWC_INPUT_STRUCT_HEADER__
#include "inputdef.hpp"
#include <array>


namespace AWC::Input {


class alignsz(64) InputUnit
{
public:
    void reset()
    {
        mouse.resetState();
        keyboard.resetState();
    }

    void create()
    {
        reset();
        mouse.previousFramePos = { DEFAULT32, DEFAULT32 };
        mouse.currentFramePos  = { DEFAULT32, DEFAULT32 };
        return;
    }


    template<typename T> 
    __force_inline std::array<T, 2> getCurrentFrameCursorPos() const {
        return mouse.getCurrentFrameCursorPos<T>();
    }
    template<typename T> 
    __force_inline std::array<T, 2> getPreviousFrameCursorPos() const {
        return mouse.getPreviousFrameCursorPos<T>();
    }
    template<typename T> 
    __force_inline std::array<T, 2> getCursorDelta() const {
        return mouse.getCursorDelta<T>();
    }
    
    __force_inline void setCurrentFrameCursorPos(std::array<f64, 2> const& xy) {
        mouse.currentFramePos = xy;
    }
    __force_inline void setPreviousFrameCursorPos(std::array<f64, 2> const& xy) {
        mouse.previousFramePos = xy;
    } 
    __force_inline void updateCursorPos(std::array<f64, 2> const& xy) {
        mouse.previousFramePos = mouse.currentFramePos;
	    mouse.currentFramePos = xy;
        return;
    }


    __force_inline inputState getKeyState(keyCode key) const {
        return keyboard.getState(__scast(u8, key));
    }
    __force_inline inputState getMouseButtonState(mouseButton key) const {
        return mouse.getState(__scast(u8, key));
    }
    __force_inline void setKeyState(keyCode key, inputState state) {
        keyboard.setState(__scast(u8, key), __scast(u8, state));
    }
    __force_inline void setMouseButtonState(mouseButton key, inputState state) {
        mouse.setState(__scast(u8, key), __scast(u8, state));
    }


private:
    typedef struct alignsz(16) KeyboardStateArray
    {
        u8 keys[(u8)keyCode::KEY_MAX + 1] = {0}; /* keyCode enum types are also used to index into the array */


        __force_inline void setState(u8 keyIndex, u8 state) {
            keys[keyIndex] = state;
        }
        __force_inline inputState getState(u8 keyIndex) const { 
            return __scast(inputState, keys[keyIndex]); 
        }
        __force_inline void resetState() {
            memset(&keys[0], 0x00, sizeof(keys));
            return;
        }
    } KeyListener;


    typedef struct alignsz(16) MouseButtonStateArray 
    {
        std::array<f64, 2> previousFramePos;
        std::array<f64, 2> currentFramePos;
        u8 buttons[static_cast<u8>(mouseButton::MAX) + 1] = {0};


        __force_inline void setState(u8 buttonIndex, u8 state) {
            buttons[buttonIndex] = state;
            return;
        }
        __force_inline inputState getState(u8 buttonIndex) const {
            return __scast(inputState, buttons[buttonIndex]); 
        }
        __force_inline void resetState() {
            memset(&buttons[0], 0x00, sizeof(buttons));
            return;
        }


        template<typename T> 
        std::array<T, 2> getCurrentFrameCursorPos() const {
            return {
                __scast(T, currentFramePos[0]),
                __scast(T, currentFramePos[1])
            };
        }
        template<typename T> 
        std::array<T, 2> getPreviousFrameCursorPos() const {
            return {
                __scast(T, previousFramePos[0]),
                __scast(T, previousFramePos[1])
            };
        }
        template<typename T> 
        std::array<T, 2> getCursorDelta() const { 
        return { /* Y axis is flipped on GLFW (X_axis = right, Y_axis = down) */
            __scast(T, (currentFramePos[0]  - previousFramePos[0]) ),
            __scast(T, (previousFramePos[1] - currentFramePos[1] ) )
            };
        }


        template<> std::array<f32, 2> getCurrentFrameCursorPos<f32>() const;
        template<> std::array<f64, 2> getCurrentFrameCursorPos<f64>() const;
        template<> std::array<u32, 2> getCurrentFrameCursorPos<u32>() const;
        template<> std::array<f32, 2> getPreviousFrameCursorPos<f32>() const;
        template<> std::array<f64, 2> getPreviousFrameCursorPos<f64>() const;
        template<> std::array<u32, 2> getPreviousFrameCursorPos<u32>() const;
        template<> std::array<f32, 2> getCursorDelta<f32>() const;
        template<> std::array<f64, 2> getCursorDelta<f64>() const;
        template<> std::array<u32, 2> getCursorDelta<u32>() const;
    } MouseListener;


private:
    MouseListener mouse;
    KeyListener   keyboard;
};


}  // namespace AWC::Input


#endif