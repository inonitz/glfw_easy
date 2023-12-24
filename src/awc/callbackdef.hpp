#ifndef __AWC_DEFAULT_CALLBACKS_DEFINITION_HEADER__
#define __AWC_DEFAULT_CALLBACKS_DEFINITION_HEADER__
#include "eventdef.hpp"
#include "util/base.hpp"


inline void glfw_error_callback(
    int         error, 
    const char* description
  ) {
    LOG_ERR_FMT("GLFW_ERROR %u - %s\n", error, description);
    return;
}


void glfw_framebuffer_size_callback(
	GLFWwindow* window, 
	int 		width, 
	int 		height
);
void glfw_key_callback(
	GLFWwindow* window, 
	int 		key, 
	int 		scancode, 
	int 		action, 
	int 		mods
);
void glfw_window_focus_callback(
	GLFWwindow* window, 
	int 		focused
);
void glfw_cursor_position_callback(
	GLFWwindow* window, 
	double 		xpos, 
	double 		ypos
);
void glfw_mouse_button_callback(
	GLFWwindow* window, 
	int 		button, 
	int 		action, 
	int 		mods
);
void gl_debug_message_callback(
	unsigned int source, 
	unsigned int type,
	unsigned int id, 
	unsigned int severity, 
	signed   int length, 
	char const*  message, 
	void const*  user_param
);


#endif