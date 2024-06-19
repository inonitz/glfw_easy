#include "render0.hpp"
#include "awc/awc.hpp"
#include "awc/usereventdef.hpp"
#include "awc/opengl.hpp"
#include "common.hpp"
#include <ImGui/imgui.h>
#include <thread>
#include "glad/gl.h"
#include "util/base.hpp"
#include "util/random.hpp"
#include "util/time.hpp"
#include "util/marker.hpp"
#include "gl/shader2.hpp"
#include "gl/texture.hpp"
#include "camera.hpp"


namespace ainput = AWC::Input;
namespace acontext = AWC::Context;


inline void custom_mousebutton_callback(user_mousebutton_struct const* data)
{
    u8 state = (ainput::inputState::PRESS == data->action && data->button == ainput::mouseButton::RIGHT);
    if(state)
        ainput::unrestrictCursor();
    else
        ainput::unlockCursor();

    return;
}


inline u8 init_awc()
{
    u8 ctxid;
    
    AWC::init();
    ifcrash( (ctxid = AWC::Context::allocate() ) == 0);
    
    AWC::Context::setActive(ctxid);
    AWC::Context::init(
        AWC::WindowOptions{{{
            WINDOW_FRAMEBUFFER_BITS_DEFAULT, 
            WINDOW_OPTION_DEFAULT | WINDOW_OPTION_RESIZABLE, 
            60, 
            0 
        }}},
        AWC::WindowDescriptor{ {{ 1024, 720 }}, nullptr }
    );
    AWC::Event::setUserCallback(&custom_mousebutton_callback);
    return ctxid;
}


template<typename T> void refresh_gpu_data(u32 bufid, std::vector<T>& buf) {
    __glcheck(
        gl()->NamedBufferData(bufid, 
            buf.size() * sizeof(buf.data()[0]), 
            buf.data(), 
            GL_DYNAMIC_DRAW
    ));
    return;
};


inline void transform_to_screen_space(std::vector<ParticleData>& buf) 
{
    const auto minus_one = math::vec2f{-1.0f};
    const auto two       = math::vec2f{2.0f};
    for(size_t i = 0; i < buf.size(); ++i) {
        buf[i].pos = minus_one + two * buf[i].pos;
        buf[i].vel = minus_one + two * buf[i].vel;
    }
    return;
}


inline void fill_particle_buffer(ParticleBuffer& buf)
{
    for(auto& p : buf) {
        p.pos = math::vec2f{ random32f(), random32f() };
        p.vel = math::vec2f{ random32f(), random32f() };
    }
    transform_to_screen_space(buf);
    return;
}


struct glState
{
    ShaderProgramV2 vertfrag;
    ShaderProgramV2 compute;
    u32 vao_id;
    u32 particleDataGPU[2];
    ColorBuffer    particleColors;
    ParticleBuffer drawBuffer;
    
    ParticleBuffer simBufferFront;
    ParticleBuffer simBufferBack;
    math::mat4f    identity;
    Camera2D       sceneCamera;
    f32            k_pradius{5.0f};

    bool updateDrawBuffer;
    bool updateShaderVertex{false};
    bool updateShaderCompute{false};
    u8   awc_id;
};


void render(
    u32 frameCount, 
    i64 frameTime, 
    i64 gameTime, 
    i64 renderTime, 
    f64 lerp,
    glState& gstate
);
void update(glState& state);




i32 render0()
{
    glState state;
    auto ctxtid = init_awc();
    bool alive{true}, paused{false};
    Time::timepoint_nano prev, curr;
    std::array<Time::timepoint_nano, 2> 
        frametime, 
        rendertime, 
        gametime, 
        updatetime, 
        lastframe, 
        lastrender, 
        lastgame;
    u32 frameCounter{0}, __unused loop, enteredUpdate;
    constexpr u32 particleAmount = 2048;
    __unused constexpr u32 minEntriesPerUpdate = 8;
    constexpr u32 targetFrameRate{60};
    constexpr f64 ms_per_frame = 1000.0f / targetFrameRate;
    constexpr f64 ns_per_frame = 1e+6f * ms_per_frame;
    const Time::nanosecond ns_per_update{__scast(i64, ns_per_frame)};
    Time::nanosecond elapsed, lag{0};
    /* 
        [NOTE]:
        ns_per_update - isn't necessarily the case that the update:frame ratio is 1:1 - 
            it actually depends on render time, as total_time - render_time = total_game_update_time
        Moreover, The program can (& Should!) separate rendering and updating game-states to 2 different concepts,
        each with differing updates/sec, depending on the performance/some other metric.
        
        total_time_per_frame = 1 / display_refresh_rate;
        render_time = measure(...);
        game_update = total_time_per_frame - render_time;
    */

    i32 max_binding_points;
    gl()->GetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &max_binding_points);
    markfmt("MAXIMUM_BINDING_POINTS_PER_SHADER => %u\n", max_binding_points);

    state.awc_id = ctxtid;
    state.vertfrag.createFrom({
        { "misc/shaders/fluid/shader.vert", GL_VERTEX_SHADER   },
        { "misc/shaders/fluid/shader.frag", GL_FRAGMENT_SHADER }
    });
    state.compute.createFrom({
        { "misc/shaders/fluid/shader0.comp", GL_COMPUTE_SHADER   },
    });
    __release_unused bool status = state.vertfrag.compile() && state.compute.compile();
    ifcrash_debug(!status);


    state.particleColors.resize(particleAmount);
    state.drawBuffer.resize(particleAmount);
    state.simBufferFront.resize(particleAmount);
    state.simBufferBack.resize(particleAmount);
    for(auto& pcol : state.particleColors) { 
        pcol = math::vec3f{ random32f(), random32f(), random32f() }; 
    }
    fill_particle_buffer(state.drawBuffer);
    transform_to_screen_space(state.drawBuffer);

    gl()->CreateVertexArrays(1, &state.vao_id);
    gl()->CreateBuffers(2, state.particleDataGPU);
    /* Vertex Array State */
    __glcheck( gl()->BindVertexArray(state.vao_id);)
    __glcheck( gl()->VertexArrayVertexBuffer(state.vao_id, 0, state.particleDataGPU[0], 0, sizeof(ParticleData));)
    __glcheck( gl()->VertexArrayVertexBuffer(state.vao_id, 1, state.particleDataGPU[1], 0, sizeof(math::vec3f));)
    __glcheck( gl()->EnableVertexArrayAttrib(state.vao_id, 0);)
    __glcheck( gl()->EnableVertexArrayAttrib(state.vao_id, 1);)
    __glcheck( gl()->EnableVertexArrayAttrib(state.vao_id, 2);)
    __glcheck( gl()->VertexArrayAttribFormat(state.vao_id, 0, 2, GL_FLOAT, false, offsetof(ParticleData, pos));)
    __glcheck( gl()->VertexArrayAttribFormat(state.vao_id, 1, 2, GL_FLOAT, false, offsetof(ParticleData, vel));)
    __glcheck( gl()->VertexArrayAttribFormat(state.vao_id, 2, 3, GL_FLOAT, true,  0);)
    __glcheck( gl()->VertexArrayAttribBinding(state.vao_id, 0, 0);)
    __glcheck( gl()->VertexArrayAttribBinding(state.vao_id, 1, 0);)
    __glcheck( gl()->VertexArrayAttribBinding(state.vao_id, 2, 1);)
    /* Vertex Buffer State */
    refresh_gpu_data(state.particleDataGPU[1], state.particleColors);
    refresh_gpu_data(state.particleDataGPU[0], state.drawBuffer);


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

        if(likely( !state.updateDrawBuffer ))
            state.updateDrawBuffer = ainput::isKeyPressed(ainput::keyCode::R) || ainput::isKeyRepeated(ainput::keyCode::R);
        if(likely( !state.updateShaderVertex )) 
            state.updateShaderVertex = ainput::isKeyPressed(ainput::keyCode::T) || ainput::isKeyRepeated(ainput::keyCode::T);

        state.k_pradius += ainput::isKeyPressed(ainput::keyCode::NUM1) || ainput::isKeyRepeated(ainput::keyCode::NUM1);
        state.k_pradius -= ainput::isKeyPressed(ainput::keyCode::NUM2) || ainput::isKeyRepeated(ainput::keyCode::NUM2);
        alive  = acontext::windowActive(ctxtid);
        alive  = alive && !ainput::isKeyPressed(ainput::keyCode::ESCAPE);
        paused = paused ^ ainput::isKeyPressed(ainput::keyCode::P);


        if(!paused) 
        {
            /* Game State Update */
            lastgame = gametime;
            gametime[0] = Time::now();
            while(lag >= ns_per_update) {
                ++enteredUpdate;
                updatetime[0] = Time::now();
                update(state);
                updatetime[1] = Time::now();
                lag -= (ns_per_update - (updatetime[1] - updatetime[0]) );
            }
            gametime[1] = Time::now();
            if(enteredUpdate != 0)
                enteredUpdate = 0;
            // if(enteredUpdate) {
            //     printf("[%2u] | %2.4f | %2.4f\n", enteredUpdate, lag.count() / 1000000.0f, ns_per_update.count() / 1000000.0f);
            //     enteredUpdate = 0;
            // }


            /* Render State Update */
            lastrender = rendertime;
            rendertime[0] = Time::now();
            state.vertfrag.bind();
            render( 
                frameCounter,
                (lastframe[1]  - lastframe[0]).count(), 
                (lastgame[1]   - lastgame[0]).count(), 
                (lastrender[1] - lastrender[0]).count(), 
                __scast(f64, lag.count()) / ns_per_frame,
                state
            );
            rendertime[1] = Time::now();
        } else {
            std::this_thread::sleep_for(ns_per_update);
        }


        /* Library State */
        AWC::end_frame();
        ++frameCounter;
        frametime[1] = Time::now();
    }


    AWC::destroy();
    return 0;
}


void render(
    u32 frameCount, 
    i64 frameTime, 
    i64 gameTime, 
    i64 renderTime, 
    f64 lerp,
    glState& gstate
) {
    if(frameCount < 10) 
        return;
    
    constexpr f64 unitsToConvert = 1e-6f; /* To Millisecond */
    f64 frameTimeDouble  = frameTime  * unitsToConvert;
    f64 gameTimeDouble   = gameTime   * unitsToConvert;
    f64 renderTimeDouble = renderTime * unitsToConvert;

    f64 fps = 1e+3f / frameTimeDouble;
    ImGui::Begin("Program Statistics");
    ImGui::Text("\
Previous Frame Statistics:\n \
Frame Counter 		 %u\n \
Frame Time 		     %3.5f [ms]\n \
  Game   State %3.5f [ms] (%2.2f%%)\n \
  Render State %3.5f [ms] (%2.2f%%)\n \
Frames Per Second    %u\n \
Interpolation Factor %3.5f (now)\n",
        frameCount, 
        frameTimeDouble, 
        gameTimeDouble, 
        100.0f * (gameTimeDouble / frameTimeDouble), 
        renderTimeDouble, 
        100.0f * (renderTimeDouble / frameTimeDouble), 
        __scast(u32, fps),
        lerp
    );
    ImGui::End();


    if(gstate.updateDrawBuffer) {
        fill_particle_buffer(gstate.drawBuffer);
        transform_to_screen_space(gstate.drawBuffer);
        refresh_gpu_data(gstate.particleDataGPU[0], gstate.drawBuffer);
        gstate.updateDrawBuffer = false;
    }
    if(gstate.updateShaderVertex) {
        gstate.vertfrag.refreshFromFiles();
        gstate.updateShaderVertex = false;
    }

    static math::vec2u winSize = AWC::Context::windowSize(gstate.awc_id);
    gstate.sceneCamera.update(lerp, math::vec2f{winSize.x, winSize.y});


    gstate.vertfrag.uniformMatrix4fv("modelmatrix", gstate.sceneCamera.getTransform());
    gstate.vertfrag.uniform1f("particleSize", gstate.k_pradius);
    /* Draw Call */
    __glcheck( gl()->DrawArrays(GL_POINTS, 0, gstate.drawBuffer.size()); );
    return;
}


void update(__unused glState& state) {
    return;
}