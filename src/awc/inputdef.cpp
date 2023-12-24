#include "inputdef.hpp"
#include <array>
#include <GLFW/glfw3.h>


namespace AWC::Input {


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


u16 toGLFWKeyCode(keyCode kc)
{
	return global_glfwKeys[static_cast<u8>(kc)];
}

keyCode toKeyCode(u16 glfw)
{
	u8 i = 0;
	while( i < (u8)keyCode::KEY_MAX && glfw != global_glfwKeys[i]) { ++i; }
	debugnobr(if(unlikely(i == (u8)keyCode::KEY_MAX)) {
		markfmt("glfwKeyToKeyCode() ==> couldn't find glfw-keyCode of value %u\n", glfw);
	});
	return (keyCode)i;
}


u16 toGLFWMouseButton(mouseButton button)
{
	return global_glfwMouseButtons[static_cast<u8>(button)];
}

mouseButton toMouseButton(u16 glfw)
{
	u8 i = 0;
	while( i < (u8)mouseButton::MAX && glfw != global_glfwMouseButtons[i]) { ++i; }
	debugnobr(if(unlikely(i == (u8)mouseButton::MAX)) {
		debug_messagefmt("glfwMouseButtonToButton() ==> couldn't find glfw-button of value %u\n", glfw);
	});
	return (mouseButton)i;
}

} // namespace AWC::Input