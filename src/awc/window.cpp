#include "window.hpp"
#include <GLFW/glfw3.h>




namespace AWC {


bool WindowContext::create(WindowDescriptor const& props, u64 windowOptions)
{
    m_props = props;
    if(!windowOptions) 
        windowOptions = WINDOW_OPTION_DEFAULT;

    WindowOptions optional;
    memcpy(&optional, &windowOptions, sizeof(u64));
    return common_create(optional);
}


bool WindowContext::create(
    u32 width, 
    u32 height, 
    std::string_view const& name,
    u64 windowOptions
) {
    m_props.x      = width;
    m_props.y      = height;
    m_props.winHdl = nullptr;
    m_props.name   = name;
    if(!windowOptions) 
        windowOptions = WINDOW_OPTION_DEFAULT;


    WindowOptions optional;
    memcpy(&optional, &windowOptions, sizeof(u64));
    return common_create(optional);
}


bool WindowContext::common_create(WindowOptions optional)
{
    /* OpenGL Context Hints */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    /* Window Specific Hints*/
    {
        glfwWindowHint(GLFW_VISIBLE,       optional.flags & 0b00000001);
        glfwWindowHint(GLFW_FOCUSED,       optional.flags & 0b00000010);
        glfwWindowHint(GLFW_CENTER_CURSOR, optional.flags & 0b00000100);
        glfwWindowHint(GLFW_RESIZABLE,     optional.flags & 0b00001000);
        glfwWindowHint(GLFW_DECORATED,     optional.flags & 0b00010000);
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
    m_props.winHdl = glfwCreateWindow(
        m_props.x, 
        m_props.y, 
        m_props.name.c_str(),
        nullptr,
        nullptr
    );
    

    return m_props.winHdl != nullptr;
}


void WindowContext::destroy()
{
    glfwMakeContextCurrent(nullptr);
    glfwDestroyWindow(m_props.winHdl);
    return;
}


void WindowContext::setCurrent() const
{
    glfwMakeContextCurrent(m_props.winHdl);
    return;
}

void WindowContext::setVerticalSync(u8 val) const 
{
    glfwSwapInterval(__scast(i32, val));
    return;
}


void WindowContext::close() const
{
    glfwSetWindowShouldClose(m_props.winHdl, GLFW_TRUE);
    return;
}


bool WindowContext::shouldClose()
{
    return glfwWindowShouldClose(m_props.winHdl);
}




}