#include "test_func.hpp"
#include "awc/awc.hpp"
#include "awc/opengl.hpp"


using namespace AWC;


int test_functionality()
{
    /* 
        Add Fail-safe to make sure no context is accidentaly initialized twice 
        Log Msg after line 53 (while loop didn't run)
        [LOG] ==> [file]: src/awc/awc.cpp [func]: glfw_error_callback [line]: 145 | GLFW_ERROR 65537 - The GLFW library is not initialized
        Clues: GLFW3 doesn't like multiple versions running, need to test on laptop too.
    */
    u8 ctxid;
    

    init();
    ctxid = Context::allocate();
    ifcrash(ctxid == 0);
    Context::setActive(ctxid);
    Context::init(
        WindowOptions{{{
            WINDOW_FRAMEBUFFER_BITS_DEFAULT, 
            WINDOW_OPTION_DEFAULT, 
            144, 
            0 
        }}},
        WindowDescriptor{ {{ 1920u, 1080u }}, nullptr }
    );


    while(Context::windowActive(ctxid))
    {
        begin_frame(); /* Begin & End Frame Are Called on the Active Context! */
        

        gl()->ClearColor(0.0f, 0.5f, 0.7f, 1.0f);
        gl()->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        /*
            Still Need to find a way to
            get a handle of the OpenGL Context so that the user can use it.


            Could be solved using an active Context + __force_inline header,
            with all of the openGL extensions defined with my active context.

            OR


            return a 'partial' Context Structure that allows access only to specific
            objects the user should be able to use, such as the GladGLContext.
        */
        end_frame();
    }
    mark();

    



    destroy();
    return 0;
}