#ifndef __AWC_EVENT_HEADER_DEFAULT_CALLBACKS__
#define __AWC_EVENT_HEADER_DEFAULT_CALLBACKS__
#include "util/base.hpp"
#include "event.hpp"



namespace AWC::Event {


extern callbackTable defaultCallbacks;


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
void glfw_mouse_button_callback(
	notused GLFWwindow* window,
	int button, 
	int action, 
	notused int mods
);
void gl_debug_message_callback(
	uint32_t 		    source, 
	uint32_t 		    type, 
	uint32_t 		    id, 
	uint32_t 		    severity, 
	notused int32_t     length, 
	char const*         message, 
    notused void const* user_param
);


} // namespace AWC::Event

#endif