#include "callbackdef.hpp"
#include "gl_instance.hpp"
#include "win_instance.hpp"
#include "input_instance.hpp"
#include "context.hpp"
#include <GLFW/glfw3.h>


using namespace AWC;


static constexpr std::array<u16, (u8)keyCode::KEY_MAX> global_glfwKeys = { 
	GLFW_KEY_ESCAPE,
	GLFW_KEY_0,
	GLFW_KEY_1,
	GLFW_KEY_2,
	GLFW_KEY_3,
	GLFW_KEY_4,
	GLFW_KEY_5,
	GLFW_KEY_6,
	GLFW_KEY_7,
	GLFW_KEY_8,
	GLFW_KEY_9,
	GLFW_KEY_W,
	GLFW_KEY_A,
	GLFW_KEY_S,
	GLFW_KEY_D,
	GLFW_KEY_Q,
	GLFW_KEY_E,
	GLFW_KEY_R,
	GLFW_KEY_F,
	GLFW_KEY_C,
	GLFW_KEY_X,
	GLFW_KEY_Z,
	GLFW_KEY_T,
	GLFW_KEY_G,
	GLFW_KEY_V,
	GLFW_KEY_B,
	GLFW_KEY_H,
	GLFW_KEY_Y,
	GLFW_KEY_U,
	GLFW_KEY_J,
	GLFW_KEY_N,
	GLFW_KEY_M,
	GLFW_KEY_K,
	GLFW_KEY_I,
	GLFW_KEY_O,
	GLFW_KEY_L,
	GLFW_KEY_P
};	


static constexpr std::array<u16, (u8)mouseButton::MAX> global_glfwMouseButtons = { 
	GLFW_MOUSE_BUTTON_LEFT,
	GLFW_MOUSE_BUTTON_RIGHT, 
	GLFW_MOUSE_BUTTON_MIDDLE 
};


constexpr u16 toGLFWKeyCode(keyCode kc)
{
	return global_glfwKeys[static_cast<u8>(kc)];
}
constexpr keyCode toKeyCode(u16 glfw)
{
	u8 i = 0;
	while( i < (u8)keyCode::KEY_MAX && glfw != global_glfwKeys[i]) { ++i; }
	debugnobr(if(unlikely(i == (u8)keyCode::KEY_MAX)) {
		markfmt("glfwKeyTokeyCode() ==> couldn't find glfw-keyCode of value %u\n", glfw);
	});
	return (keyCode)i;
}


constexpr u16 toGLFWMouseButton(mouseButton button)
{
	return global_glfwMouseButtons[static_cast<u8>(button)];
}
constexpr mouseButton toMouseButton(u16 glfw)
{
	u8 i = 0;
	while( i < (u8)mouseButton::MAX && glfw != global_glfwMouseButtons[i]) { ++i; }
	debugnobr(if(unlikely(i == (u8)mouseButton::MAX)) {
		debug_messagefmt("glfwmouseButtonToButton() ==> couldn't find glfw-button of value %u\n", glfw);
	});
	return (mouseButton)i;
}


inline auto* __curr_gl()     { return getCurrentlyActiveGLContext(); }
inline auto* __curr_winptr() { return getCurrentlyActiveWindow();    }
inline auto* __curr_io()     { return Input::getCurrentlyActiveInputUnit(); }




void glfw_framebuffer_size_callback(
	GLFWwindow* window, 
	int 		width, 
	int 		height
) {
    u8 minimized, size_change;
    auto* descriptor = __rcast(WindowDescriptor*, __curr_winptr());
    
    __curr_gl()->Viewport(0, 0, width, height);
    minimized   = (width == 0) || (height == 0);
    size_change = (descriptor->x != width) || (descriptor->y != height);

	debug_messagefmt("Framebuffer Size Callback to [ %dx%d ] , Old -> [ %ux%u ]\n", 
        width, height, 
        descriptor->x, descriptor->y
    );

    SET_BIT_AT(descriptor->config.status, 0, minimized);
    SET_BIT_AT(descriptor->config.status, 1, !minimized && size_change);
    descriptor->x = width;
    descriptor->y = height;
    return;
}


void glfw_key_callback(
	GLFWwindow* window, 
	int key, 
	int scancode, 
	int action, 
	int mods
) {
    static std::array<const char*, (u8)inputState::MAX + 1> actionStr = {
		"RELEASED",
		"PRESSED ",
		"REPEAT  ",
		""
	};
    auto* input_state = __curr_io();
	keyCode    keyCodeIndex;
    inputState before;
    

    keyCodeIndex = toKeyCode(key);
    debugnobr(
        before = input_state->getKeyState(keyCodeIndex)
    );

	actionStr[3] = actionStr[static_cast<u8>(action)];
    input_state->setKeyState(keyCodeIndex, __scast(inputState, (1 << action)) );
	

	debug_messagefmt("[key_callback][kci=%02hhu][Before=%hhu]  [%s] Key %s  [After=%hhu]\n", 
		keyCodeIndex, 
		before, 
		glfwGetKeyName(key, scancode), 
		actionStr[3], 
		input_state->getKeyState(keyCodeIndex)
	);
	return;
}


void glfw_window_focus_callback(
	GLFWwindow* window, 
	int 		focused
) {
    return;
}


void glfw_cursor_position_callback(
	GLFWwindow* window, 
	double 		xpos, 
	double 		ypos
) {
    auto* input_state = __curr_io();
    input_state->updateCursorPos({ xpos, ypos });
	return;
}


void glfw_mouse_button_callback(
	GLFWwindow* window, 
	int 		button, 
	int 		action, 
	int 		mods
) {
    static std::array<const char*, (u8)inputState::MAX + 1> actionStr = {
		"RELEASED",
		"PRESSED ",
		"REPEAT  ",
		""
	};
	static std::array<const char*, (u8)mouseButton::MAX + 2> ButtonNames = {
		"MOUSE_BUTTON_LEFT  ",
		"MOUSE_BUTTON_RIGHT ",
		"MOUSE_BUTTON_MIDDLE",
		"MOUSE_BUTTON_UNKOWN",
		""
	};
    auto* input_state = __curr_io();
    mouseButton buttonIndex;
    inputState before;
    

    buttonIndex = toMouseButton(button); /* might return MoueButton::MAX */
    debugnobr(
        before = input_state->getMouseButtonState(buttonIndex);
    );

	actionStr[3]   = actionStr[static_cast<u8>(action)];
	ButtonNames[4] = ButtonNames[static_cast<u8>(buttonIndex)];
    input_state->setMouseButtonState(buttonIndex, __scast(inputState, (1 << action) ));

	if(Input::isMouseButtonPressed(mouseButton::RIGHT)) 
        Input::lockCursor();

	if(Input::isMouseButtonPressed(mouseButton::LEFT)) 
        Input::unlockCursor();
	

	debug_messagefmt("[mouse_button_callback][bi=%02hhu][Before=%hhu]  [%s] Mouse Button %s  [After=%hhu]\n", 
		buttonIndex,
		before,
		ButtonNames[4],
		actionStr[3], 
        input_state->getMouseButtonState(buttonIndex)
	);
	return;
}


#ifdef _DEBUG
void gl_debug_message_callback(
	unsigned int source, 
	unsigned int type,
	unsigned int id, 
	unsigned int severity, 
	signed   int length, 
	char const*  message, 
	void const*  user_param
) {
	const std::pair<u32, const char*> srcStr[6] = {
		{ GL_DEBUG_SOURCE_API,             "API" 			 },
		{ GL_DEBUG_SOURCE_WINDOW_SYSTEM,   "WINDOW SYSTEM"   },
		{ GL_DEBUG_SOURCE_SHADER_COMPILER, "SHADER COMPILER" },
		{ GL_DEBUG_SOURCE_THIRD_PARTY,	   "THIRD PARTY" 	 },
		{ GL_DEBUG_SOURCE_APPLICATION,	   "APPLICATION" 	 },
		{ GL_DEBUG_SOURCE_OTHER, 		   "OTHER" 			 }
	};
	const std::pair<u32, const char*> typeStr[7] = {
		{ GL_DEBUG_TYPE_ERROR, 			     "ERROR"               },
		{ GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "DEPRECATED_BEHAVIOR" },
		{ GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,  "UNDEFINED_BEHAVIOR"  },
		{ GL_DEBUG_TYPE_PORTABILITY,		 "PORTABILITY" 	       },
		{ GL_DEBUG_TYPE_PERFORMANCE,		 "PERFORMANCE" 		   },
		{ GL_DEBUG_TYPE_MARKER,			   	 "MARKER" 			   },
		{ GL_DEBUG_TYPE_OTHER,			     "OTHER" 			   }
	};
	const std::pair<u32, const char*> severityStr[6] = {
		{ GL_DEBUG_SEVERITY_NOTIFICATION, "NOTIFICATION" },
		{ GL_DEBUG_SEVERITY_LOW, 		  "LOW"		     },
		{ GL_DEBUG_SEVERITY_MEDIUM, 	  "MEDIUM"	     },
		{ GL_DEBUG_SEVERITY_HIGH, 		  "HIGH"	     }
	};
	const char* src_str      = srcStr[0].second;
	const char* type_str     = typeStr[0].second;
	const char* severity_str = severityStr[0].second;
	u32 		idx 		 = 0;
	

	while(srcStr[idx].first != source) { ++idx; }
	src_str = srcStr[idx].second;
	idx = 0;

	while(typeStr[idx].first != type)  { ++idx; }
	type_str = typeStr[idx].second;
	idx = 0;

	while(severityStr[idx].first != severity)  { ++idx; }
	severity_str = severityStr[idx].second;
	idx = 0;
	
	
	printf("OPENGL >> %s::%s::%s %u: %s\n", src_str, type_str, severity_str, id, message);
	return;
}
#else
void gl_debug_message_callback(
	__unused GLenum 	   source, 
	__unused GLenum 	   type, 
	__unused GLuint 	   id, 
	__unused GLenum 	   severity, 
	__unused GLsizei  	   length, 
	__unused GLchar const* message, 
	__unused void const*   user_param
) {
	return;
}
#endif