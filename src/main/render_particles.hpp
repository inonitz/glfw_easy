#ifndef __RENDER_PARTICLE_BUFFER__
#define __RENDER_PARTICLE_BUFFER__
#include "awc/awc.hpp"
#include "util/ifcrash.hpp"


int render_particles(); /* Main Function */


inline u8 init_lib()
{
    u8 ctxid;
    
    AWC::init();
    ifcrash( (ctxid = AWC::Context::allocate() ) == NULL);
    
    AWC::Context::setActive(ctxid);
    AWC::Context::init(
        AWC::WindowOptions{{{
            WINDOW_FRAMEBUFFER_BITS_DEFAULT, 
            WINDOW_OPTION_DEFAULT | WINDOW_OPTION_RESIZABLE, 
            144, 
            0 
        }}},
        AWC::WindowDescriptor{ {{ 1920u, 1080u }}, nullptr }
    );
    return ctxid;
}

#endif