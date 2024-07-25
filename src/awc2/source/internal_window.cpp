#include "internal_window.hpp"
#include "GLFW/glfw3.h"
#include "awc2/include/window_types.hpp"
#include <cstring>


namespace AWC2::internal {

bool Window::create(
    u16 width, 
    u16 height, 
    u64 windowOptions,
    GLFWwindow* win_share
) {
    WindowDescriptor windesc;
    memcpy(&windesc, &windowOptions, sizeof(decltype(windesc)));


    /* OpenGL Context Hints */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    /* Window Specific Hints*/
    glfwWindowHint(GLFW_VISIBLE, __scast(i32, windesc.createFlags & 
        WindowCreationFlag::STARTUP_VISIBLE) 
    );
    glfwWindowHint(GLFW_FOCUSED, __scast(i32, windesc.createFlags & 
        WindowCreationFlag::STARTUP_FOCUSED) 
    );
    glfwWindowHint(GLFW_CENTER_CURSOR, __scast(i32, windesc.createFlags & 
        WindowCreationFlag::STARTUP_CENTER_CURSOR) 
    );
    glfwWindowHint(GLFW_RESIZABLE, __scast(i32, windesc.createFlags & 
        WindowCreationFlag::RESIZABLE) 
    );
    glfwWindowHint(GLFW_DECORATED, __scast(i32, windesc.createFlags & 
        WindowCreationFlag::BORDER) 
    );
    glfwWindowHint(GLFW_REFRESH_RATE, windesc.refreshRate == 0 
        ? GLFW_DONT_CARE : windesc.refreshRate
    );
    glfwWindowHint(GLFW_DEPTH_BITS,   (windesc.framebufferChannels >> 25) & 0b11111);
    glfwWindowHint(GLFW_STENCIL_BITS, (windesc.framebufferChannels >> 20) & 0b11111);
    glfwWindowHint(GLFW_RED_BITS,     (windesc.framebufferChannels >> 15) & 0b11111);
    glfwWindowHint(GLFW_GREEN_BITS,   (windesc.framebufferChannels >> 10) & 0b11111);
    glfwWindowHint(GLFW_BLUE_BITS,    (windesc.framebufferChannels >> 5 ) & 0b11111);
    glfwWindowHint(GLFW_ALPHA_BITS,   (windesc.framebufferChannels >> 0 ) & 0b11111);
    m_data = {
        windesc,
        glfwCreateWindow(
            __scast(i32, width), 
            __scast(i32, height), 
            "Window ", 
            nullptr, 
            win_share
        ),
        win_share,
        width,
        height
    };
    return m_data.handle == nullptr;
}


void Window::destroy() {
    glfwDestroyWindow(m_data.handle);
    return;
}
void Window::setCurrent() const {
    glfwMakeContextCurrent(m_data.handle);
    return;
}
void Window::swapBuffers() const {
    glfwSwapBuffers(m_data.handle);
    return;
}
void Window::setVerticalSync(u8 val) const {
    glfwSwapInterval(__scast(i32, val));
    return;
}
void Window::close() const {
    glfwSetWindowShouldClose(m_data.handle, true);
    return;
}
bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_data.handle);
}


} // namespace AWC2::internal