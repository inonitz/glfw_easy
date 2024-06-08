#ifndef __AWC_USER_EVENT_CALLBACK_DEFINITION__
#define __AWC_USER_EVENT_CALLBACK_DEFINITION__
#include "util/base.hpp"
#include <array>
#include "inputdef.hpp"


typedef struct GLFWwindow GLFWwindow;


struct user_winsize_struct 
{
	GLFWwindow* window;
	u32 width;
	u32 height;
};
struct user_keyboard_struct
{
	GLFWwindow* window;
	AWC::Input::keyCode    keyStroke;
	AWC::Input::inputState action;
};
struct user_winfocus_struct 
{
	GLFWwindow* window;
	bool focused;
};
struct user_mousecursor_struct 
{
	GLFWwindow* window;
	std::array<f64, 2> pos;
};
struct user_mousescroll_struct 
{
	GLFWwindow* window;
	std::array<f64, 2> offset;
};
struct user_mousebutton_struct 
{
	GLFWwindow* window;
	AWC::Input::mouseButton button;
	AWC::Input::inputState  action;
};


typedef void (*user_callback_noop		 )(void* generic_pointer);
typedef void (*user_callback_window_size )(user_winsize_struct const*);
typedef void (*user_callback_keyboard	 )(user_keyboard_struct const*);
typedef void (*user_callback_window_focus)(user_winfocus_struct const*);
typedef void (*user_callback_mouse_pos	 )(user_mousecursor_struct const*);
typedef void (*user_callback_mouse_scroll)(user_mousescroll_struct const*);
typedef void (*user_callback_mouse_button)(user_mousebutton_struct const*);


inline void user_callback_func_noop(notused void* ptr) { 
	return; 
}

#endif