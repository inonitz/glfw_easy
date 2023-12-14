#ifndef __AWC_EVENT_DEFINITION_HEADER__
#define __AWC_EVENT_DEFINITION_HEADER__


typedef struct GLFWwindow GLFWwindow;
typedef void (* GLFWerrorfun)(
	int 		    error_code, 
	const char* description
);
typedef void (* GLFWframebuffersizefun)(
	GLFWwindow* window, 
	int 		width, 
	int 		height
);
typedef void (* GLFWkeyfun)(
	GLFWwindow* window, 
	int 		key, 
	int 		scancode, 
	int 		action, 
	int 		mods
);
typedef void (* GLFWwindowfocusfun)(
	GLFWwindow* window, 
	int 		focused
);
typedef void (* GLFWcursorposfun)(
	GLFWwindow* window, 
	double 		xpos, 
	double 		ypos
);
typedef void (* GLFWmousebuttonfun)(
	GLFWwindow* window, 
	int 		button, 
	int 		action, 
	int 		mods
);
typedef void (* OpenGLdbgmsgfun)(
	unsigned int source, 
	unsigned int type,
	unsigned int id, 
	unsigned int severity, 
	signed   int length, 
	char const*  message, 
	void const*  user_param
);

#endif