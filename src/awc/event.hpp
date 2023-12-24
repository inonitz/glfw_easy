#ifndef __AWC_EVENT_HEADER__
#define __AWC_EVENT_HEADER__
#include "util/base.hpp"
#include "eventdef.hpp"


namespace AWC::Event {


struct alignsz(64) callbackTable 
{
    using generic_error    = GLFWerrorfun;
    using framebuffer_size = GLFWframebuffersizefun;
    using input_keys       = GLFWkeyfun;
	using window_focused   = GLFWwindowfocusfun;
	using mouse_position   = GLFWcursorposfun;
	using mouse_input      = GLFWmousebuttonfun;


    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnested-anon-types"
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
    union 
    {
        struct {
            // generic_error    errorEvent;
            framebuffer_size windowSizeEvent;
            input_keys       keyEvent;
            window_focused   activeWinEvent;
            mouse_position   mousePosEvent;
            mouse_input      mouseButtonEvent;
#ifdef _DEBUG
            OpenGLdbgmsgfun  openglDebugEvent = nullptr; /* if nullptr context doesn't have opengl context */
#endif
            u64 reserved0[3 - _DEBUG];
        };
        struct {
            u64 pointers[6 - _DEBUG];
            u64 reserved1[2  + _DEBUG];
        };
    };
    #pragma GCC diagnostic pop
    #pragma GCC diagnostic pop
};


}

#endif