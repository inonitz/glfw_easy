#ifndef __AWC2_EVENT_INTERFACE_HEADER__
#define __AWC2_EVENT_INTERFACE_HEADER__
#include "input_types.hpp"


typedef struct GLFWwindow GLFWwindow;


namespace AWC2
{
    struct user_callback_winsize_struct 
    {
        GLFWwindow* window;
        u32 width;
        u32 height;
    };
    struct user_callback_keyboard_struct
    {
        GLFWwindow* window;
        AWC2::Input::keyCode    keyStroke;
        AWC2::Input::inputState action;
    };
    struct user_callback_winfocus_struct 
    {
        GLFWwindow* window;
        bool focused;
    };
    struct user_callback_mousecursor_struct 
    {
        GLFWwindow* window;
        AWC2::Input::cursorPosition64 pos;
    };
    struct user_callback_mousescroll_struct 
    {
        GLFWwindow* window;
        AWC2::Input::cursorPosition64 offset;
    };
    struct user_callback_mousebutton_struct 
    {
        GLFWwindow* window;
        AWC2::Input::mouseButton button;
        AWC2::Input::inputState  action;
    };


    typedef void (*user_callback_noop		 )(void* generic_pointer);
    typedef void (*user_callback_window_size )(user_callback_winsize_struct const*);
    typedef void (*user_callback_keyboard	 )(user_callback_keyboard_struct const*);
    typedef void (*user_callback_window_focus)(user_callback_winfocus_struct const*);
    typedef void (*user_callback_mouse_pos	 )(user_callback_mousecursor_struct const*);
    typedef void (*user_callback_mouse_scroll)(user_callback_mousescroll_struct const*);
    typedef void (*user_callback_mouse_button)(user_callback_mousebutton_struct const*);


    template<typename Func> void setUserCallback(Func const&);
}; // namespace AWC2


#endif