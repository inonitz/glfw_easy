#include "def_callback.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "awc_internal.hpp"
#include "awc.hpp"
#include "input.hpp"
#include "event.hpp"
#include "window.hpp"


namespace AWC::Event {


using generic_state = AWC::Input::inputState;
using generic_key   = AWC::Input::keyCode;
using generic_mbut  = AWC::Input::mouseButton;


callbackTable defaultCallbacks = {
	{{
		glfw_framebuffer_size_callback,
		glfw_key_callback,
		glfw_window_focus_callback,
		glfw_cursor_position_callback,
		glfw_mouse_button_callback,
#if _OPENGL_DEBUG_FLAG == 1
		gl_debug_message_callback,
		{0, 0}
#else
		{0, 0, 0}
#endif
	}}
};

void glfw_error_callback(int error, const char* description);


#define WINDOW_FLAG_MINIMIZED_SHIFT 5
#define WINDOW_FLAG_SIZE_CHANGE_SHIFT 6
void glfw_framebuffer_size_callback(
	notused GLFWwindow* handle,
	i32 w, 
	i32 h
) {
    auto& active 		= AWC::activeContext();
	auto& activeWinData = active.win->data();
	bool minimized, sizeChange;
	
	active.opengl->Viewport(0, 0, w, h);
	minimized  = (w == 0) || (h == 0);
	sizeChange = !minimized && ( activeWinData.desc.x != __scast(u32, w) || activeWinData.desc.y != __scast(u32, h) );


	AWC_LIB_MODIFY_VAR_BITS(activeWinData.flags.flags,
		(1 << WINDOW_FLAG_MINIMIZED_SHIFT),
		minimized << WINDOW_FLAG_MINIMIZED_SHIFT
	);
	AWC_LIB_MODIFY_VAR_BITS(activeWinData.flags.flags,
		(1 << WINDOW_FLAG_SIZE_CHANGE_SHIFT),
		sizeChange << WINDOW_FLAG_SIZE_CHANGE_SHIFT
	);
	// activeWinData.flags.flags &= ~(1 << WINDOW_FLAG_MINIMIZED_SHIFT); 	   /* reset the bit */
	// activeWinData.flags.flags |= minimized << WINDOW_FLAG_MINIMIZED_SHIFT; /*  set  the bit */
	// activeWinData.flags.flags &= ~(1 << WINDOW_FLAG_SIZE_CHANGE_SHIFT);
	// activeWinData.flags.flags |= sizeChange << WINDOW_FLAG_SIZE_CHANGE_SHIFT;
	debug_messagefmt("Framebuffer Callback => %dx%d [Old was %ux%u]\n", 
		w, 
		h, 
		activeWinData.desc.x, 
		activeWinData.desc.y
	);
	activeWinData.desc.x = __scast(u32, w);
	activeWinData.desc.y = __scast(u32, h);
	return;
}
#undef WINDOW_FLAG_MINIMIZED_SHIFT
#undef WINDOW_FLAG_SIZE_CHANGE_SHIFT


void glfw_key_callback(
	notused GLFWwindow* handle,
	int key, 
	notused int scancode, 
	int action, 
	notused int mods
) {
	static std::array<const char*, (u8)generic_state::MAX + 1> actionStr = {
		"RELEASED",
		"PRESSED ",
		"REPEAT  ",
		""
	};
	
	
	auto& active = AWC::activeContext();
	generic_key keyCodeIndex = AWC::Input::toKeyCode(key);
	debugnobr(
		u8 before = __scast(u8, 
			active.unit->getKeyState(keyCodeIndex)
		);
	);
	
	actionStr[3] = actionStr[static_cast<u8>(action)];
	active.unit->setKeyState(keyCodeIndex, (1 << action));
	

	debug_messagefmt("[key_callback][kci=%02hhu][Before=%u]  [%s] Key %s  [After=%u]\n", 
		keyCodeIndex, 
		before,
		glfwGetKeyName(key, scancode), 
		actionStr[3], 
		__scast(u8, active.unit->getKeyState(keyCodeIndex) )
	);
	return;
}


#define WINDOW_FLAG_FOCUSED_SHIFT 7
void glfw_window_focus_callback(
	notused GLFWwindow* window,
	int 				focused
) {
	auto& activeWinData = AWC::activeContext().win->data();
	AWC_LIB_RESET_SET_BITS(activeWinData.flags.flags, 
		(1 << WINDOW_FLAG_FOCUSED_SHIFT), 
		boolean(focused) << WINDOW_FLAG_FOCUSED_SHIFT
	);
	// activeWinData.flags.flags &= ~(1 << WINDOW_FLAG_FOCUSED_SHIFT); 	        /* reset the bit */
	// activeWinData.flags.flags |= boolean(focused) << WINDOW_FLAG_FOCUSED_SHIFT; /*  set  the bit */
	return;
}
#undef WINDOW_FLAG_FOCUSED_SHIFT


void glfw_cursor_position_callback(
	notused GLFWwindow* window,
	double xpos, 
	double ypos
) {
	auto& active = AWC::activeContext();
	active.unit->updateMousePosition({ 
		__scast(f32, xpos), 
		__scast(f32, ypos) 
	});
	return;
}


void glfw_mouse_button_callback(
	notused GLFWwindow* window,
	int button, 
	int action, 
	notused int mods
) {
	static std::array<const char*, (u8)generic_state::MAX + 1> actionStr = {
		"RELEASED",
		"PRESSED ",
		"REPEAT  ",
		""
	};
	static std::array<const char*, (u8)generic_mbut::MAX + 2> ButtonNames = {
		"MOUSE_BUTTON_LEFT  ",
		"MOUSE_BUTTON_RIGHT ",
		"MOUSE_BUTTON_MIDDLE",
		"MOUSE_BUTTON_UNKOWN",
		""
	};


	auto& active = AWC::activeContext();	
	generic_mbut buttonIndex = AWC::Input::toMouseButton(button); /* might return MoueButton::MAX */
	debugnobr(
		u8 before = __scast(u8,
			active.unit->getMouseButtonState(buttonIndex)
		);
	)

	actionStr[3]   = actionStr[static_cast<u8>(action)];
	ButtonNames[4] = ButtonNames[static_cast<u8>(buttonIndex)];
	active.unit->setMouseButtonState(buttonIndex, (1 << action));


	if(AWC::Input::isMouseButtonPressed(generic_mbut::RIGHT))
		AWC::Input::lockCursor();
	if(AWC::Input::isMouseButtonPressed(generic_mbut::LEFT))
		AWC::Input::unlockCursor();

	debug_messagefmt("[mouse_button_callback][bi=%02hhu][Before=%u]  [%s] Mouse Button %s  [After=%u]\n", 
		buttonIndex,
		before,
		ButtonNames[4],
		actionStr[3],
		__scast(u8, active.unit->getMouseButtonState(buttonIndex) )
	);
	return;  
}


#ifdef _DEBUG
void gl_debug_message_callback(
	uint32_t 		  	source, 
	uint32_t 		  	type, 
	uint32_t 		  	id, 
	uint32_t 		  	severity, 
	notused int32_t     length, 
	char const*         message, 
	notused void const* user_param
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
	notused uint32_t 	source, 
	notused uint32_t 	type, 
	notused uint32_t 	id, 
	notused uint32_t 	severity, 
	notused int32_t     length, 
	notused char const* message, 
	notused void const* user_param
) {
	return;
}
#endif


} // namespace AWC::Event