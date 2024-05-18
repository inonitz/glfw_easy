#include "render_new.hpp"
#include "util/marker.hpp"
#include "awc/awc.hpp"
#include "awc/opengl.hpp"
#include "util/time.hpp"


namespace ainput = AWC::Input;
namespace acontext = AWC::Context;


u8 __init()
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


i32 render_new()
{
    mark(); auto ctxtid = __init();
    u32 loop; f32 lerp;
    bool alive{true};
    constexpr u32 targetProgramTick = 32;
    constexpr u32 skipTicks = 1024 / targetProgramTick;
    constexpr u32 maxFrameSkip = 4;
    Time::timepoint_nano current;
    Time::nanosecond skip_tick_per_frame{skipTicks};
    
    
    auto update = []() -> void {};
    auto render = [](notused f32 dt) -> void {};


    mark(); gl()->Enable(GL_PROGRAM_POINT_SIZE);
    mark(); gl()->Enable(GL_BLEND); 
    mark(); gl()->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    mark(); gl()->ClearColor(0.0f, 0.5f, 0.7f, 1.0f);
    mark(); current = Time::now();
    mark(); while (alive)
    {
        AWC::begin_frame();


        loop = 0;
        while(Time::now() > current && loop < maxFrameSkip) {
            update();

            current += skip_tick_per_frame;
            ++loop;
        }
        lerp = __scast(f32, 
            (Time::now() - current + skip_tick_per_frame ) / skip_tick_per_frame
        );
        render(lerp);


        alive = acontext::windowActive(ctxtid);
        alive = alive && ainput::isKeyPressed(ainput::keyCode::ESCAPE);
        AWC::end_frame();
    }


    AWC::destroy();
    return 0;
}