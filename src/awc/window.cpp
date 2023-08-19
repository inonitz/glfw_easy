#include "window.hpp"
#include <GLFW/glfw3.h>
#include "context.hpp"



namespace AWC {


void WindowContext::create(WindowDescriptor const& props, WindowOptions const& optional = {})
{
    m_props = props;
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
        glfwWindowHint(GLFW_REFRESH_RATE,  optional.refresh_rate == 0 
            ? GLFW_DONT_CARE : optional.refresh_rate
        );
        glfwWindowHint(GLFW_STENCIL_BITS, optional.framebuffer_flags.stencil);
        glfwWindowHint(GLFW_DEPTH_BITS,   optional.framebuffer_flags.depth  );
        glfwWindowHint(GLFW_RED_BITS,     optional.framebuffer_flags.rgba[0]);
        glfwWindowHint(GLFW_GREEN_BITS,   optional.framebuffer_flags.rgba[1]);
        glfwWindowHint(GLFW_BLUE_BITS,    optional.framebuffer_flags.rgba[2]);
        glfwWindowHint(GLFW_ALPHA_BITS,   optional.framebuffer_flags.rgba[3]);

    }
    debugnobr(
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    )
    m_props.winHdl = glfwCreateWindow(
        m_props.x, 
        m_props.y, 
        m_props.name,
        nullptr,
        nullptr
    );
    
    ifcrashdo(!m_props.winHdl, {
        AWC::destroy();
    });
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