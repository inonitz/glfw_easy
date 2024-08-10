#include "include/awc2.hpp"
#include "include/context.hpp"
#include "include/event.hpp"
#include "include/input_types.hpp"
#include <glbinding/gl/gl.h>


void fbo_callback(__unused AWC2::user_callback_winsize_struct const* winsize) {
    return;
}
void key_callback(AWC2::user_callback_keyboard_struct const* key) {    
    if(key->keyStroke == AWC2::Input::keyCode::ESCAPE && key->action == AWC2::Input::inputState::PRESS) {
        AWC2::closeContext(key->context_id);
    }
    return;
}



int render_awc2()
{
    AWC2::init();
    
    
    __unused u32 framecount{0};
    f32 framecountf{0};
    AWC2::ContextVector contexts = AWC2::getActiveContextList();
    AWC2::ContextID context_id[3] = {
        AWC2::createContext(),
        AWC2::createContext(),
        AWC2::createContext()
    };


    AWC2::initializeContext(context_id[0], 640, 480, AWC2::WindowDescriptor{});
    AWC2::initializeContext(context_id[1], 640, 480, AWC2::WindowDescriptor{});
    AWC2::initializeContext(context_id[2], 640, 480, AWC2::WindowDescriptor{});
    for(auto ctx : contexts) {
        AWC2::setContextUserCallback(ctx, &fbo_callback);
        AWC2::setContextUserCallback(ctx, &key_callback);
    }

    
    while(!contexts.empty())
    {
        AWC2::newFrame();
        for(auto& active_ctx_id : contexts) {
            ++framecountf;
            AWC2::setCurrentContext(active_ctx_id);
            AWC2::begin();
            gl::glClearColor(1.0f, 0.0f, framecountf / 255.0f, 0.0f);
            gl::glClear(gl::ClearBufferMask{gl::GL_COLOR_BUFFER_BIT});
            AWC2::end();
        }
        framecountf = (framecountf > 255.0f) ? 0.0f : framecountf;
    }


    for(u8 i = 0; i < __carraysize(context_id); ++i) {
        AWC2::destroyContext(context_id[i]);
    }
    AWC2::destroy();
    return 0;
}