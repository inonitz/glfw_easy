#include "window.hpp"
#include "event.hpp"
#include "windowdef.hpp"
#include <GLFW/glfw3.h>




namespace AWC {


bool WindowContext::create(
    WindowDescriptor const& props, 
    u64                     windowOptions,
    GLFWwindow*             shared_win
) {
    m_data.desc = props;
    WindowOptions optional; optional.bits = windowOptions;
    return common_create(optional, shared_win);
}


bool WindowContext::create(
    u32 width, 
    u32 height, 
    u64 windowOptions,
    GLFWwindow* sharedwin
) {
    m_data.desc = WindowDescriptor{ {{ width, height }}, nullptr };
    WindowOptions optional; optional.bits = windowOptions;
    return common_create(optional, sharedwin);
}


bool WindowContext::common_create(WindowOptions optional, GLFWwindow* shared_win)
{
    /* OpenGL Context Hints */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    /* Window Specific Hints*/
    {
        glfwWindowHint(GLFW_VISIBLE,       optional.flags & WINDOW_OPTION_STARTUP_VISIBLE);
        glfwWindowHint(GLFW_FOCUSED,       optional.flags & WINDOW_OPTION_STARTUP_FOCUSED);
        glfwWindowHint(GLFW_CENTER_CURSOR, optional.flags & WINDOW_OPTION_STARTUP_CENTER_CURSOR);
        glfwWindowHint(GLFW_RESIZABLE,     optional.flags & WINDOW_OPTION_RESIZABLE);
        glfwWindowHint(GLFW_DECORATED,     optional.flags & WINDOW_OPTION_BORDER);
        glfwWindowHint(GLFW_REFRESH_RATE,  optional.refresh == 0 
            ? GLFW_DONT_CARE : optional.refresh
        );
        glfwWindowHint(GLFW_STENCIL_BITS, (optional.fb_channels >> 0 ) & 0b11111);
        glfwWindowHint(GLFW_DEPTH_BITS,   (optional.fb_channels >> 5 ) & 0b11111);
        glfwWindowHint(GLFW_RED_BITS,     (optional.fb_channels >> 10) & 0b11111);
        glfwWindowHint(GLFW_GREEN_BITS,   (optional.fb_channels >> 15) & 0b11111);
        glfwWindowHint(GLFW_BLUE_BITS,    (optional.fb_channels >> 20) & 0b11111);
        glfwWindowHint(GLFW_ALPHA_BITS,   (optional.fb_channels >> 25) & 0b11111);

    }
    debugnobr(
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    )
    m_data.desc.winHdl = glfwCreateWindow(
        m_data.desc.x, 
        m_data.desc.y, 
        "Window - OpenGL 4.6 Multi-Context",
        nullptr,
        shared_win
    );
    if ( (m_data.desc.winHdl != nullptr) && 
        (optional.flags & WINDOW_OPTION_RAW_MOUSE_MOTION) && 
        glfwRawMouseMotionSupported()
    ) {
        glfwSetInputMode(m_data.desc.winHdl, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    return m_data.desc.winHdl != nullptr;
}


void WindowContext::destroy()
{
    glfwMakeContextCurrent(nullptr);
    glfwDestroyWindow(m_data.desc.winHdl);
    return;
}


void WindowContext::setCurrent() const
{
    glfwMakeContextCurrent(m_data.desc.winHdl);
    return;
}

void WindowContext::setVerticalSync(u8 val) const 
{
    glfwSwapInterval(__scast(i32, val));
    return;
}


void WindowContext::setEventHooks(const Event::callbackTable* const hooks) const {
    const AWC::Event::callbackTable* table = hooks;
    glfwSetWindowSizeCallback (underlying_handle(), table->windowSizeEvent );
    glfwSetKeyCallback        (underlying_handle(), table->keyEvent        );
    glfwSetWindowFocusCallback(underlying_handle(), table->activeWinEvent  );
    glfwSetCursorPosCallback  (underlying_handle(), table->mousePosEvent   );
    glfwSetMouseButtonCallback(underlying_handle(), table->mouseButtonEvent);
    glfwSetScrollCallback     (underlying_handle(), table->mouseScrollEvent);
    return;
}


void WindowContext::close() const
{
    glfwSetWindowShouldClose(m_data.desc.winHdl, GLFW_TRUE);
    return;
}


bool WindowContext::shouldClose()
{
    return glfwWindowShouldClose(m_data.desc.winHdl);
}


bool WindowContext::isMinimized() const
{
    return boolean( (m_data.cfg.flags >> 5) & 1);
}


bool WindowContext::sizeChanged() const
{
    return boolean( (m_data.cfg.flags >> 6) & 1);
}


bool WindowContext::isFocused() const
{
    return boolean( (m_data.cfg.flags >> 7) & 1);
}


} // namespace AWC