#include "internal_callback.hpp"
#include "awc2/include/input_types.hpp"
#include "awc2/include/window_types.hpp"
#include "awc2/source/internal_instance.hpp"
#include "awc2/source/internal_state.hpp"

#include "util/marker2.hpp"
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <GLFW/glfw3.h>


#define __call_user_callback_func(func_type, __args) __rcast(func_type, \
			__awc2_lib_get_active_context()->event_table.pointers[UserFuncIndexer<func_type>()()] \
		)(&__args);


namespace AWC2::internal {


void glfw_framebuffer_size_callback(notused GLFWwindow* handle, i32 w, i32 h) 
{
	auto& win_data = __awc2_lib_get_active_context()->window.m_data;
	glbinding::useContext(__awc2_lib_get_active_context_id());
	gl::glViewport(0, 0, w, h);


    bool minimized = ( (w == 0) || (h == 0) );
    bool sizeChange = !minimized && ( 
			win_data.width  != __scast(u16, w) || 
			win_data.height != __scast(u16, h) 
		);
    

    win_data.description.stateFlags &= ~(WindowStateFlag::MINIMIZED | WindowStateFlag::SIZE_CHANGED);
    win_data.description.stateFlags |= (
        from_conditional(WindowStateFlag::SIZE_CHANGED, sizeChange) 
        | 
        from_conditional(WindowStateFlag::MINIMIZED, minimized)
    );
	user_callback_winsize_struct __funcargs{handle, __scast(u32, w) , __scast(u32, h) };
	__call_user_callback_func(user_callback_window_size, __funcargs);


	markfmt("[framebuffer_callback][Before=%ux%i]  Window Size Changed  [After=%ux%u]\n",
		win_data.width, 
		win_data.height,
		w, h
	);
	win_data.width  = __scast(u16, w);
	win_data.height = __scast(u16, h);
	return;
}


void glfw_key_callback(
	notused GLFWwindow* handle,
	int key, 
	notused int scancode, 
	int action, 
	notused int mods
) {
	static std::array<const char*, (u8)Input::inputState::MAX + 1> actionStr = {
		"RELEASED",
		"PRESSED ",
		"REPEAT  ",
		""
	};
	
	
    auto& key_state = __awc2_lib_get_active_context()->io;
	Input::keyCode keyCodeIndex = AWC2::internal::toKeyCode(key);
	debugnobr(
		u8 before = __scast(u8, 
			key_state.getKeyState(keyCodeIndex)
		);
	);
	actionStr[3] = actionStr[static_cast<u8>(action)];
	key_state.setKeyState(keyCodeIndex, (1 << action));


	user_callback_keyboard_struct __funcargs{handle, keyCodeIndex, __scast(Input::inputState, (1 << action) ) };
	__call_user_callback_func(user_callback_keyboard, __funcargs);
	

	const char* key_name = glfwGetKeyName(key, scancode);
	key_name = (key_name == nullptr) ? AWC2::internal::keyCodeToString(keyCodeIndex) : key_name;
	markfmt("[key_callback][kci=%02hhu][Before=%u]  [%s]  Key %s  [After=%u]\n", 
		__scast(u8, keyCodeIndex),
		before,
		actionStr[3],
		key_name, 
		__scast(u8, active.getKeyState(keyCodeIndex) )
	);
	return;
}


void glfw_window_focus_callback(
	notused GLFWwindow* window,
	int 				focused
) {
	auto& win_data = __awc2_lib_get_active_context()->window.m_data;
	debugnobr(
		static const std::array<const char*, 4> actionStr = {
			"UNFOCUSED",
			"FOCUSED  ",
			"Unfocused",
			"Focused  "
		};
		u8 before = __scast(bool, win_data.description.stateFlags & WindowStateFlag::FOCUSED),
			after = boolean(focused);
	)
    win_data.description.stateFlags &= ~WindowStateFlag::FOCUSED;
    win_data.description.stateFlags |= from_conditional(WindowStateFlag::FOCUSED, focused);
	
    user_callback_winfocus_struct __funcargs{window, __scast(bool, focused) };
	__call_user_callback_func(user_callback_window_focus, __funcargs);


	markfmt("[window_focus_callback][fi=%02hhu][Before=%u]  [%s]  Window %s  [After=%u]\n",
		__scast(u8, focused),
		before,
		actionStr[after],
		actionStr[after + 2],
		after
	);
	return;
}


void glfw_cursor_position_callback(
	notused GLFWwindow* window,
	double xpos, 
	double ypos
) {
	auto& mouse_state = __awc2_lib_get_active_context()->io;
    mouse_state.updateMousePosition({ 
		__scast(f32, xpos), 
		__scast(f32, ypos) 
	});
	user_callback_mousecursor_struct __funcargs{window, {{{xpos, ypos}}} };
	__call_user_callback_func(user_callback_mouse_pos, __funcargs);
	return;
}


void glfw_scroll_offset_callback(
	notused GLFWwindow* window,
	double xoffset,
	double yoffset
) {
	auto& mouse_state = __awc2_lib_get_active_context()->io;
	mouse_state.updateScrollOffset({ 
		__scast(f32, xoffset), 
		__scast(f32, yoffset) 
	});
	user_callback_mousescroll_struct __funcargs{window, {{{ xoffset, yoffset }}} };
	__call_user_callback_func(user_callback_mouse_scroll, __funcargs);
	return;	
}


void glfw_mouse_button_callback(
	notused GLFWwindow* window,
	int button, 
	int action, 
	notused int mods
) {
	static std::array<const char*, __scast(u8, Input::mouseButton::MAX) + 1> actionStr = {
		"RELEASED",
		"PRESSED ",
		"REPEAT  ",
		""
	};
	static std::array<const char*, __scast(u8, Input::mouseButton::MAX) + 2> ButtonNames = {
		"MOUSE_BUTTON_LEFT  ",
		"MOUSE_BUTTON_RIGHT ",
		"MOUSE_BUTTON_MIDDLE",
		"MOUSE_BUTTON_UNKOWN",
		""
	};


	auto& mouse_state = __awc2_lib_get_active_context()->io;
    Input::mouseButton buttonIndex = AWC2::internal::toMouseButton(button); /* might return MoueButton::MAX */
	debugnobr(
		u8 before = __scast(u8,
			mouse_state.getMouseButtonState(buttonIndex)
		);
	)
	actionStr[3]   = actionStr[static_cast<u8>(action)];
	ButtonNames[4] = ButtonNames[static_cast<u8>(buttonIndex)];
	mouse_state.setMouseButtonState(buttonIndex, (1 << action));


	user_callback_mousebutton_struct __funcargs{window, buttonIndex, __scast(Input::inputState, (1 << action) ) };
	__call_user_callback_func(user_callback_mouse_button, __funcargs);

	markfmt("[mouse_button_callback][bi=%02hhu][Before=%u]  [%s]  Mouse Button %s  [After=%u]\n", 
		__scast(u8, buttonIndex),
		before,
		ButtonNames[4],
		actionStr[3],
		__scast(u8, mouse_state.getMouseButtonState(buttonIndex) )
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
	const std::pair<gl::GLenum, const char*> srcStr[6] = {
		{ gl::GL_DEBUG_SOURCE_API,             "API" 			 },
		{ gl::GL_DEBUG_SOURCE_WINDOW_SYSTEM,   "WINDOW SYSTEM"   },
		{ gl::GL_DEBUG_SOURCE_SHADER_COMPILER, "SHADER COMPILER" },
		{ gl::GL_DEBUG_SOURCE_THIRD_PARTY,	   "THIRD PARTY" 	 },
		{ gl::GL_DEBUG_SOURCE_APPLICATION,	   "APPLICATION" 	 },
		{ gl::GL_DEBUG_SOURCE_OTHER, 		   "OTHER" 			 }
	};
	const std::pair<gl::GLenum, const char*> typeStr[7] = {
		{ gl::GL_DEBUG_TYPE_ERROR, 			     "ERROR"               },
		{ gl::GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "DEPRECATED_BEHAVIOR" },
		{ gl::GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,  "UNDEFINED_BEHAVIOR"  },
		{ gl::GL_DEBUG_TYPE_PORTABILITY,		 "PORTABILITY" 	       },
		{ gl::GL_DEBUG_TYPE_PERFORMANCE,		 "PERFORMANCE" 		   },
		{ gl::GL_DEBUG_TYPE_MARKER,			   	 "MARKER" 			   },
		{ gl::GL_DEBUG_TYPE_OTHER,			     "OTHER" 			   }
	};
	const std::pair<gl::GLenum, const char*> severityStr[6] = {
		{ gl::GL_DEBUG_SEVERITY_NOTIFICATION, "NOTIFICATION" },
		{ gl::GL_DEBUG_SEVERITY_LOW, 		  "LOW"		     },
		{ gl::GL_DEBUG_SEVERITY_MEDIUM, 	  "MEDIUM"	     },
		{ gl::GL_DEBUG_SEVERITY_HIGH, 		  "HIGH"	     }
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