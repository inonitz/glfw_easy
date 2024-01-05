#include "test_func.hpp"
#include "awc/awc.hpp"
#include "awc/opengl.hpp"


int test_functionality()
{
    mark(); AWC::init();
    
    
    mark(); u8 ctxid = AWC::Context::allocate();
    mark(); ifcrash(ctxid == 0);
    mark(); AWC::Context::setActive(ctxid);
    mark(); AWC::Context::init(
        AWC::WindowOptions{{{
            WINDOW_FRAMEBUFFER_BITS_DEFAULT, 
            WINDOW_OPTION_DEFAULT, 
            144, 
            0 
        }}},
        AWC::WindowDescriptor{ {{ 1920u, 1080u }}, nullptr }
    );


    while(AWC::Context::childWindowActive(ctxid))
    {
        AWC::begin_frame(); /* Begin & End Frame Are Called on the Active Context! */
        

        gl::cinst()->ClearColor(0.0f, 0.5f, 0.7f, 1.0f);
        gl::cinst()->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        /*
            Still Need to find a way to
            get a handle of the OpenGL Context so that the user can use it.


            Could be solved using an active Context + __force_inline header,
            with all of the openGL extensions defined with my active context.

            OR


            return a 'partial' AWC::Context Structure that allows access only to specific
            objects the user should be able to use, such as the GladGLContext.
        */

        AWC::end_frame();
    }

    



    AWC::destroy();
    return 0;
}