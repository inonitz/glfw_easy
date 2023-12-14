#ifndef __AWC_INPUT_STRUCT_HEADER__
#define __AWC_INPUT_STRUCT_HEADER__
#include "inputdef.hpp"
#include <array>


namespace AWC::Input {


class alignsz(64) InputUnit
{
public:
    void create();
    void reset();
    void onUpdate();


    template<typename T> 
    __force_inline std::array<T, 2> getCurrentFrameCursorPos() {
        return mouse.getCurrentFrameCursorPos<T>();
    }
    template<typename T> 
    __force_inline std::array<T, 2> getPreviousFrameCursorPos() {
        return mouse.getPreviousFrameCursorPos<T>();
    } 
    template<typename T> 
    __force_inline std::array<T, 2> getCursorDelta() {
        return mouse.getCursorDelta<T>();
    }

    inputState getKeyState        (keyCode key    ) const;
    inputState getMouseButtonState(mouseButton key) const;


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