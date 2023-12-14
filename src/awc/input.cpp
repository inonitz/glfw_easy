#include "input.hpp"
#include "instance.hpp"
#include <GLFW/glfw3.h>


namespace AWC::Input {


void InputUnit::create()
{
    reset();
    mouse.previousFramePos = { DEFAULT32, DEFAULT32 };
    mouse.currentFramePos  = { DEFAULT32, DEFAULT32 };
    return;
}

void InputUnit::onUpdate()
{
    mouse.previousFramePos = mouse.currentFramePos;
    glfwGetCursorPos(
        getActiveContext().win->underlying_handle(), 
        &mouse.currentFramePos[0], 
        &mouse.currentFramePos[1]
    );
    return;
}

void InputUnit::reset()
{
    mouse.resetState();
    keyboard.resetState();
}


} // namespace AWC::Input