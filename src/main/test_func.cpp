#include "test_func.hpp"
#include "awc/awc.hpp"
#include "awc/opengl.hpp"
#include "util/count.hpp"
#include   "util/time.hpp"


using namespace AWC;


int test_functionality()
{
    /* 
        Add Fail-safe to make sure no context is accidentaly initialized twice 
    */
    Time::Timer clock;
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

    
    /* 
        Input handling (besides maybe mouse input) is not active, since 
        I haven't hooked GLFW to the event handlers yet.
        find a convenient & sensible way to add a 'finalize'/'hook'
        function at the end of event-handler updates.
    */
    while(Context::windowActive(ctxid))
    {
        clock.tick();
        begin_frame(); /* Begin & End Frame Are Called on the Active Context! */
        
        
        gl()->ClearColor(0.0f, 0.5f, 0.7f, 1.0f);
        gl()->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        end_frame();
        clock.tock();
        // markfmt("frame took %llu [ms]", clock.duration());
    }


    destroy();
    return 0;
}