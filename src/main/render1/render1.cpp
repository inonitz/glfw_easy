#include "render1.hpp"
#include "awc/awc.hpp"
#include "awc/usereventdef.hpp"
#include "awc/opengl.hpp"
#include "glad/gl.h"
#include "util/random.hpp"
#include "util/count.hpp"
#include "util/time.hpp"
#include "gl/shader2.hpp"
#include <ImGui/imgui.h>
#include <thread>


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
            144, 
            0 
        }}},
        AWC::WindowDescriptor{ {{ 1024, 720 }}, nullptr }
    );
    AWC::Event::setUserCallback(&custom_mousebutton_callback);
    return ctxid;
}




namespace Render {


typedef struct compute_shader_simulation_constants 
{
    f32 dt;
    f32 viscosity;
    f32 initialDensity;
    f32 densityFactor;
    math::vec2f gravity;
    math::vec2f delta;
    math::vec2f inv_delta;
    math::vec2i dims;
} computeConstants;


typedef struct __measuring_program_performance
{
    u32 m_frameCount;
    i64 m_frameTime;
    i64 m_gameTime;
    i64 m_renderTime;
    f64 m_interpolate_frame;
} frameTimeData;


struct glState {
    u32 m_fluidtex[3];
    u32 m_fboid;
    u32  m_ubocompute;
    bool m_updateShaderCompute{false};
    bool m_swapTextures;
    bool reserved[2];
    ShaderProgramV2 m_compute;


    void prepare(math::vec2i& sim_bounds);
};


struct ProgramState
{
    computeConstants sim_params;
    glState          graphics;
    frameTimeData    timing;
    u32              code_block_counter{0};
    u8               awc_context_id;
    u8               reserved[3];
};


void renderImGui(ProgramState& glob_state);
void render(ProgramState& glob_state);
void update(ProgramState& glob_state);


} // namespace Render


i32 render1()
{
    Render::ProgramState globalState;
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
    u32 frameCounter{0}, __unused loop;
    __unused constexpr u32 minEntriesPerUpdate = 8;
    constexpr u32 targetFrameRate{144};
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

    globalState.awc_context_id = init_awc();
    globalState.sim_params = Render::computeConstants{
        0.006944444f,
        0.2f,
        1.0f,
        0.3f,
        math::vec2f{-9.8f},
        math::vec2f{1.0f},
        math::vec2f{1.0f},
        math::vec2i{512, 512}
    };
    globalState.graphics.prepare(globalState.sim_params.dims);
    globalState.graphics.m_compute.bind();


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


        alive  = acontext::windowActive(globalState.awc_context_id);
        alive  = alive && !ainput::isKeyPressed(ainput::keyCode::ESCAPE);
        paused = paused ^ ainput::isKeyPressed(ainput::keyCode::P);
        if(!paused) 
        {
            /* Game State Update */
            lastgame = gametime;
            gametime[0] = Time::now();
            while(lag >= ns_per_update) {
                updatetime[0] = Time::now();
                Render::update(globalState);
                updatetime[1] = Time::now();
                lag -= (ns_per_update - (updatetime[1] - updatetime[0]) );
            }
            gametime[1] = Time::now();


            /* Render State Update */
            lastrender = rendertime;
            rendertime[0] = Time::now();
            globalState.timing = {
                frameCounter,
                (lastframe[1]  - lastframe[0]).count(), 
                (lastgame[1]   - lastgame[0]).count(), 
                (lastrender[1] - lastrender[0]).count(), 
                __scast(f64, lag.count()) / ns_per_frame,
            };
            Render::render(globalState);
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




namespace Render {


void glState::prepare(math::vec2i& sim_bounds)
{
    m_compute.createFrom({
        { "src/main/render1/shader0.comp", GL_COMPUTE_SHADER },
    });
    __release_unused bool status = m_compute.compile();
    ifcrash_debug(!status);


    __glcheck( gl()->CreateTextures(GL_TEXTURE_2D, 3, m_fluidtex));
    __glcheck( gl()->CreateBuffers(1, &m_ubocompute));
    __glcheck( gl()->CreateFramebuffers(1, &m_fboid));

    /* Buffer Configuration & Allocation */
    /* fluidtex[0] & fluidtex[1] are constantly swapped and serve as I/O for the compute shader */
    __glcheck( gl()->TextureParameteri(m_fluidtex[0], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    __glcheck( gl()->TextureParameteri(m_fluidtex[0], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    __glcheck( gl()->TextureParameteri(m_fluidtex[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    __glcheck( gl()->TextureParameteri(m_fluidtex[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    __glcheck( gl()->TextureStorage2D(m_fluidtex[0], 1, GL_RGBA32F, sim_bounds.x, sim_bounds.y));
    __glcheck( gl()->TextureParameteri(m_fluidtex[1], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    __glcheck( gl()->TextureParameteri(m_fluidtex[1], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    __glcheck( gl()->TextureParameteri(m_fluidtex[1], GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    __glcheck( gl()->TextureParameteri(m_fluidtex[1], GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    __glcheck( gl()->TextureStorage2D(m_fluidtex[1], 1, GL_RGBA32F, sim_bounds.x, sim_bounds.y));
    /* fluidtex[2] serves as the visual representation of the velocity field - the actual drawing part */
    __glcheck( gl()->TextureParameteri(m_fluidtex[2], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    __glcheck( gl()->TextureParameteri(m_fluidtex[2], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    __glcheck( gl()->TextureParameteri(m_fluidtex[2], GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    __glcheck( gl()->TextureParameteri(m_fluidtex[2], GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    __glcheck( gl()->TextureStorage2D(m_fluidtex[2], 1, GL_RGBA32F, sim_bounds.x, sim_bounds.y));
    __glcheck( gl()->NamedBufferStorage(m_ubocompute, sizeof(computeConstants), &m_ubocompute, GL_DYNAMIC_STORAGE_BIT));

    __glcheck( gl()->BindBufferRange(GL_UNIFORM_BUFFER, 5, m_ubocompute, 0, sizeof(computeConstants)));
    m_compute.UniformBlock("SimulationConstants", 5);
    __glcheck( gl()->NamedFramebufferTexture(m_fboid, GL_COLOR_ATTACHMENT0, m_fluidtex[2], 0));
    __glcheck( gl()->BindImageTexture(2, m_fluidtex[2], 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F));


    u32 fbstatus;
    while( ( fbstatus = gl()->CheckNamedFramebufferStatus(m_fboid, GL_FRAMEBUFFER) ) != GL_FRAMEBUFFER_COMPLETE) {}


    /* Fill Texture with 0'th iteration data */
    std::vector<math::vec4f> initialData{__scast(u64, sim_bounds.x * sim_bounds.y)};
    for(auto& p : initialData) {
        p = { random32f(), random32f(), 1.0f, 0.0f };
    }
    __glcheck( gl()->TextureSubImage2D(m_fluidtex[0], 0, 0, 0, sim_bounds.x, sim_bounds.y, GL_RGBA, GL_FLOAT, initialData.data()));
    __glcheck( gl()->NamedBufferSubData(m_ubocompute, 0, sizeof(Render::computeConstants), &m_ubocompute)); /* Upload UBO to compute shader */
    return;
}


void renderImGui(ProgramState& state)
{
    constexpr f64 unitsToConvert = 1e-6f; /* To Millisecond */
    f64 frameTimeDouble  = state.timing.m_frameTime  * unitsToConvert;
    f64 gameTimeDouble   = state.timing.m_gameTime   * unitsToConvert;
    f64 renderTimeDouble = state.timing.m_renderTime * unitsToConvert;

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
        state.timing.m_frameCount, 
        frameTimeDouble, 
        gameTimeDouble, 
        100.0f * (gameTimeDouble / frameTimeDouble), 
        renderTimeDouble, 
        100.0f * (renderTimeDouble / frameTimeDouble), 
        __scast(u32, fps),
        state.timing.m_interpolate_frame
    );
    ImGui::End();


    return;
}


void render(ProgramState& state) {
    auto& gfx       = state.graphics;
    auto& sim_state = state.sim_params;

    /* Self Explanatory */
    renderImGui(state);

    /* Update Graphics State */
    if(gfx.m_updateShaderCompute) {
        gfx.m_compute.refreshFromFiles();
        gfx.m_updateShaderCompute = false;
    }
    if(gfx.m_swapTextures) {
        u32 tmp = gfx.m_fluidtex[0];
        gfx.m_fluidtex[0] = gfx.m_fluidtex[1];
        gfx.m_fluidtex[1] = tmp;

        __glcheck( gl()->BindImageTexture(0, gfx.m_fluidtex[0], 0, false, 0, GL_READ_ONLY, GL_RGBA32F));
        __glcheck( gl()->BindImageTexture(1, gfx.m_fluidtex[1], 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F));
        gfx.m_swapTextures = false;
    }

    /* Dispatch Compute Shader */
    __rconce(state.code_block_counter,
        gfx.m_compute.resizeLocalWorkGroup(0, { 1, 1, 1 }); 
    )
    __glcheck( gl()->DispatchCompute(sim_state.dims.x, sim_state.dims.y, 1));
    __glcheck( gl()->MemoryBarrier(GL_ALL_BARRIER_BITS));


    /* Draw Call */
    static const auto winSize = acontext::windowSize(state.awc_context_id);
    __glcheck( gl()->BindFramebuffer(GL_READ_FRAMEBUFFER, gfx.m_fboid));
    __glcheck( gl()->BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
    __glcheck( gl()->BlitFramebuffer(0, 0, sim_state.dims.x, sim_state.dims.y, 0, 0, winSize[0], winSize[1], GL_COLOR_BUFFER_BIT, GL_LINEAR));
    // __glcheck( gl()->BlitNamedFramebuffer(gfx.draw, 0, 
    //     0, 0, sim_state.dims.x, sim_state.dims.y, 
    //     0, 0, winSize[0], winSize[1],
    //     GL_COLOR_BUFFER_BIT, 
    //     GL_LINEAR
    // ));
    gfx.m_swapTextures = true;
    
    
    return;
}


void update(__unused ProgramState& state) {
    return;
}


} // namespace Render