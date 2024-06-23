#include "render_new.hpp"
#include "ImGui/imgui.h"
#include "awc/awc.hpp"
#include "awc/opengl.hpp"
#include "util/time.hpp"


namespace ainput = AWC::Input;
namespace acontext = AWC::Context;


u8 __init()
{
    u8 ctxid;
    
    AWC::init();
    ifcrash( (ctxid = AWC::Context::allocate() ) == 0);
    
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
    auto ctxtid = __init();
    __unused u32 frameCounter{0}; 
    bool alive{true};
    std::array<Time::timepoint_nano, 2> frametime, rendertime, gametime, updatetime, lastframe, lastrender, lastgame;
    Time::timepoint_nano prev, curr;
    constexpr u32 targetFrameRate{144};
    constexpr f64 ms_per_frame = 1000.0f / targetFrameRate;
    constexpr f64 ns_per_frame = 1e+6f * ms_per_frame;
    const Time::nanosecond ns_per_update{__scast(i64, ns_per_frame)};/* Really depends on render time, total_time - render_time = total_game_update_time */ 
    Time::nanosecond elapsed, lag{0};
    /* 
        total_time_per_frame = 1 / display_refresh_rate;
        render_time = measure(...);
        game_update = total_time_per_frame - render_time;
        Moreover, given a refresh rate R for the monitor,
        the program might not necessarily have to update according to that;
        Simply put, we can separate rendering and updating to 2 different concepts,
        each with differing updates/sec, depending on the performance/some other metric.
    */


    auto l_update = []() -> void {};
    auto l_render = [](
        u32 frameCount, 
        f32 frameTime, 
        f32 gameTime, 
        f32 renderTime, 
        f64 lerp) -> void 
    {
        if(frameCount < 10) 
            return;
        
        constexpr f32 unitsToConvert = 1e+3f; /* Millisecond */
        frameTime *= unitsToConvert;
        gameTime *= unitsToConvert;
        renderTime *= unitsToConvert;

        f32 fps = unitsToConvert/frameTime;
        ImGui::Begin("Program Statistics");
        ImGui::Text("\
            Previous Frame Statistics:\n \
            Frame Counter 		 %u\n \
            Frame Time 		     %3.5f (ms)\n \
                Game   State %3.5f (%2.2f%%)\n \
                Render State %3.5f (%2.2f%%)\n \
            Frames Per Second    %u\n \
            Interpolation Factor %3.5f (now)\n",
            frameCount, 
            frameTime, 
            gameTime, 
            100 * (gameTime / frameTime), 
            renderTime, 
            100 * (renderTime / frameTime), 
            __scast(u32, fps),
            lerp
        );
        ImGui::End();
    };


    gl()->Enable(GL_PROGRAM_POINT_SIZE);
    gl()->Enable(GL_BLEND); 
    gl()->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    gl()->ClearColor(0.0f, 0.5f, 0.7f, 1.0f);
    prev = Time::now();
    while(alive)
    {
        lastframe = frametime;
        frametime[0] = Time::now();

        /* Counters */
        curr = Time::now();
        elapsed = curr - prev;
        prev = curr;
        lag += elapsed;
        AWC::begin_frame();


        /* Game State Update */
        lastgame = gametime;
        gametime[0] = Time::now();
        while(lag >= ns_per_update) {
            updatetime[0] = Time::now();
            l_update();
            updatetime[1] = Time::now();
            lag -= (ns_per_update - (updatetime[1] - updatetime[0]) );
        }
        gametime[1] = Time::now();


        /* Render State Update */
        lastrender = rendertime;
        rendertime[0] = Time::now();
        l_render( 
            frameCounter,
            Time::dursecondf32{lastframe[1] - lastframe[0]}.count(), 
            Time::dursecondf32{lastgame[1] - lastgame[0]}.count(), 
            Time::dursecondf32{lastrender[1] - lastrender[0]}.count(), 
            __scast(f64, lag.count()) / ns_per_frame
        );
        rendertime[1] = Time::now();


        /* Window/Library State */
        alive = acontext::windowActive(ctxtid);
        alive = alive && !ainput::isKeyPressed(ainput::keyCode::ESCAPE);
        AWC::end_frame();
        ++frameCounter;


        frametime[1] = Time::now();
    }


    AWC::destroy();
    return 0;
}