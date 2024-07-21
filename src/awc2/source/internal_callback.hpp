#ifndef __AWC2_INTERNAL_DEFAULT_CALLBACK_HEADER__
#define __AWC2_INTERNAL_DEFAULT_CALLBACK_HEADER__
#include "util/macro.hpp"


typedef struct GLFWwindow GLFWwindow;


namespace AWC2::internal {


void glfw_error_callback(
    int error, 
    const char* description
);
void glfw_framebuffer_size_callback(
	notused GLFWwindow* handle,
	int w, 
	int h
);
void glfw_key_callback(
	notused GLFWwindow* handle,
	int key,
	notused int scancode,
	int action,
	notused int mods
);
void glfw_window_focus_callback(
	GLFWwindow* window,
	int 		focused
);
void glfw_cursor_position_callback(
	notused GLFWwindow* window,
	double xpos,
	double ypos
);
void glfw_scroll_offset_callback(
	notused GLFWwindow* window,
	double xoffset,
	double yoffset
);
void glfw_mouse_button_callback(
	notused GLFWwindow* window,
	int button, 
	int action, 
	notused int mods
);
void gl_debug_message_callback(
	unsigned int 		source, 
	unsigned int 		type, 
	unsigned int 		id, 
	unsigned int 		severity, 
	notused signed int  length, 
	char const*         message, 
    notused void const* user_param
);


} // namespace AWC2::internal


#endif