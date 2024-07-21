#include "internal_window.hpp"
#include "GLFW/glfw3.h"



namespace AWC2::internal {

bool Window::create(
    u16 width, 
    u16 height, 
    u64 windowOptions,
    GLFWwindow* multi_window_opengl_context_share
) {

}
bool Window::create(
    WindowDescriptor const& props,
    GLFWwindow* multi_window_opengl_context_share
) {

}
void Window::destroy() {

}
void Window::setCurrent() const {

}
void Window::swapBuffers() const {

}
void Window::setVerticalSync(u8 val) const {
    
}
void Window::close() const {
    glfwSetWindowShouldClose(m_data.handle, true);
    return;
}
bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_data.handle);
}


} // namespace AWC2::internal