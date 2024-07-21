#include "fastfluid.hpp"
#include <glbinding/gl46core/gl.h>
#include <GLFW/glfw3.h>
#include "awc/awc.hpp"
#include "awc/def_callback.hpp"
#include "util/marker2.hpp"


namespace AWCIN = AWC::Input;


int render_awc()
{
    bool alive{true};
    u8 ctx[2] = { 0, 0 };


    AWC::init();
    ctx[0] = AWC::Context::allocate();
    ctx[1] = AWC::Context::allocate();
    AWC::Context::init(ctx[0],
        AWC::WindowOptions{{{
            WINDOW_OPTION_FRAMEBUFFER_BITS_DEFAULT, 
            WINDOW_OPTION_STARTUP_VISIBLE | 
            WINDOW_OPTION_STARTUP_CENTER_CURSOR | 
            WINDOW_OPTION_RESIZABLE | 
            WINDOW_OPTION_BORDER | 
            WINDOW_OPTION_RAW_MOUSE_MOTION,
            60, 
            0 
        }}},
        AWC::WindowDescriptor{ {{ 640, 360 }}, nullptr },
        AWC::Event::defaultCallbacks
    );
    AWC::Context::init(ctx[1],
        AWC::WindowOptions{{{
            WINDOW_OPTION_FRAMEBUFFER_BITS_DEFAULT, 
            WINDOW_OPTION_STARTUP_VISIBLE | 
            WINDOW_OPTION_STARTUP_CENTER_CURSOR | 
            WINDOW_OPTION_RESIZABLE | 
            WINDOW_OPTION_BORDER | 
            WINDOW_OPTION_RAW_MOUSE_MOTION,
            60, 
            0 
        }}},
        AWC::WindowDescriptor{ {{ 640, 360 }}, nullptr },
        AWC::Event::defaultCallbacks
    );


    markstr("\n");
    while(alive) 
    {
        mark(); AWC::begin_frame();
        mark(); AWC::Context::setActive(ctx[0]);
        mark(); AWC::Context::begin();
        mark(); gl46core::glClear(gl::ClearBufferMask{GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT});
        mark(); AWC::Context::end();
        
        mark(); AWC::Context::setActive(ctx[1]);
        mark(); AWC::Context::begin();
        mark(); gl46core::glClear(gl::ClearBufferMask{GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT});
        mark(); AWC::Context::end();

        mark(); alive  = !AWC::Context::shouldClose(ctx[0]) && !AWC::Context::shouldClose(ctx[1]);
        mark(); alive  &= !AWCIN::isKeyPressed(AWCIN::keyCode::ESCAPE);
        mark(); AWC::end_frame();
    }


    AWC::destroy();
    return 0;
}