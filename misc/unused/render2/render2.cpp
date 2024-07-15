#include "render2.hpp"
#include "awc/awc.hpp"
#include "awc/usereventdef.hpp"
#include "awc/opengl.hpp"
#include "glad/gl.h"
#include "util/marker.hpp"
#include "util/random.hpp"
#include "util/time.hpp"
#include "gl/shader2.hpp"
#include "util/vec.hpp"
#include <ImGui/imgui.h>
#include <_mingw_mac.h>
#include <thread>
#include <filesystem>
#include <utility>


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




namespace ProgramRender {


typedef struct compute_shader_simulation_constants 
{
    f32 viscosity;
    f32 initialDensity;
    f32 densityFactor;
    math::vec2f gravity;
    math::vec2f delta;
    math::vec2f inv_delta;
    math::vec2i dims;
} computeConstants;


typedef struct compute_shader_clamp_delta_time
{
    f32 dt;
    f32 reserved;
    math::vec2f unitDistanceInv;
    math::vec2f maxSimVelocity;
} clampDeltaTime;


typedef struct compute_shader_particle_buffer_definition
{
    struct ParticleData {
        math::vec4f position;
        math::vec4f color;
    };

    u32          particleCount;
    u32          reserved[7];
    ParticleData buffer[1];
} ParticleBuffer;


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
    u32 m_ubocompute;
    u32 m_ssboparticle;
    u32 m_ssbodt;
    ShaderProgramV2 m_computeSim;
    ShaderProgramV2 m_computeVisual;


    void prepare(
        math::vec2i&            sim_bounds, 
        computeConstants const* forBlockBuffers,
        clampDeltaTime const*   deltaTimeLimiter
    );
};


struct ProgramState
{
    computeConstants sim_params;
    clampDeltaTime   delta_time;
    glState          graphics;
    frameTimeData    timing;
    u32              code_block_counter{0};
    u8               awc_context_id;
    u8               reserved[3];
};


void renderImGui(ProgramState& glob_state);
void render(ProgramState& glob_state);
void update(ProgramState& glob_state);


} // namespace ProgramRender


i32 render2()
{
    ProgramRender::ProgramState globalState;
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
    globalState.sim_params = ProgramRender::computeConstants{
        0.2f,
        1.0f,
        0.3f,
        math::vec2f{-9.8f},
        math::vec2f{1.0f},
        math::vec2f{1.0f},
        math::vec2i{512, 512},
    };
    globalState.delta_time = ProgramRender::clampDeltaTime{
        0.016666667f, /* 0.006944444f, */
        0.0f,
        globalState.sim_params.inv_delta,
        math::vec2f{0.0f}
    };
    globalState.graphics.prepare(globalState.sim_params.dims, &globalState.sim_params, &globalState.delta_time);

    markfmt("Original => { m_fluidtex[0]: %u | m_fluidtex[1]: %u }", globalState.graphics.m_fluidtex[0], globalState.graphics.m_fluidtex[1]);
    std::swap(globalState.graphics.m_fluidtex[0], globalState.graphics.m_fluidtex[1]);



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
                ProgramRender::update(globalState);
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
            ProgramRender::render(globalState);
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




namespace ProgramRender {


void glState::prepare(
    math::vec2i&            sim_bounds, 
    computeConstants const* simParamForBlockBuffers, 
    clampDeltaTime   const* deltaTimeLimiter
) {
    static constexpr const char* shaderName[2] = { "compute.comp", "visual.comp" };
    static const std::string shaderPath[2] = {
        ( std::filesystem::current_path()/std::filesystem::path{std::string{"src/main/render2/"}}/std::filesystem::path{shaderName[0]} ).generic_u8string(),
        ( std::filesystem::current_path()/std::filesystem::path{std::string{"src/main/render2/"}}/std::filesystem::path{shaderName[1]} ).generic_u8string()
    };

    /* First-Time Memory Init */
    /* Fill Texture with 0'th iteration data */
    std::vector<math::vec4f> initialData{__scast(u64, sim_bounds.x * sim_bounds.y)};
    for(auto& p : initialData) {
        p = { random32f(), random32f(), 1.0f, 0.0f };
    }

    /* First-Time Memory Init */
    constexpr u32 particleCount = 256;
    constexpr size_t bufferSize = sizeof(ParticleBuffer) + sizeof(ParticleBuffer::ParticleData) * ( particleCount - 1);
    ParticleBuffer* initialParticles = __rcast(ParticleBuffer*, malloc(bufferSize));
    initialParticles->particleCount = particleCount;
    for(u32 i = 0; i < particleCount; ++i) {
        initialParticles->buffer[i].position = { random32f() * sim_bounds.x, random32f() * sim_bounds.y, 0.0f, 0.0f };
        initialParticles->buffer[i].color = { 0.0f, 0.0f, 1.0f, 1.0f };
    }


    m_computeSim.createFrom({ { shaderPath[0].data(), GL_COMPUTE_SHADER } });
    m_computeSim.resizeLocalWorkGroup(0, { 1, 1, 1 });
    m_computeVisual.createFrom({ { shaderPath[1].data(), GL_COMPUTE_SHADER } });
    m_computeVisual.resizeLocalWorkGroup(0, { 1, 1, 1 });
    __release_unused bool status = m_computeSim.compile();
    ifcrash_debug(!status);
    status = m_computeVisual.compile();
    ifcrash_debug(!status);


    __glcheck( gl()->CreateTextures(GL_TEXTURE_2D, 3, m_fluidtex));
    __glcheck( gl()->CreateBuffers(1, &m_ubocompute));
    __glcheck( gl()->CreateBuffers(1, &m_ssboparticle));
    __glcheck( gl()->CreateBuffers(1, &m_ssbodt));
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
    

    /* FBO Config & Image-Bind to outpos */
    __glcheck( gl()->NamedFramebufferTexture(m_fboid, GL_COLOR_ATTACHMENT0, m_fluidtex[2], 0));
    u32 fbstatus;
    while(  ( fbstatus = gl()->CheckNamedFramebufferStatus(m_fboid, GL_FRAMEBUFFER) ) != GL_FRAMEBUFFER_COMPLETE  ) {}


    __glcheck( gl()->TextureSubImage2D(m_fluidtex[0], 0, 0, 0, sim_bounds.x, sim_bounds.y, GL_RGBA, GL_FLOAT, initialData.data()));
    __glcheck( gl()->NamedBufferStorage(m_ubocompute, sizeof(ProgramRender::computeConstants), simParamForBlockBuffers, GL_DYNAMIC_STORAGE_BIT));
    __glcheck( gl()->NamedBufferStorage(m_ssbodt, sizeof(ProgramRender::clampDeltaTime), deltaTimeLimiter, GL_DYNAMIC_STORAGE_BIT));
    __glcheck( gl()->NamedBufferStorage(m_ssboparticle, bufferSize, initialParticles, GL_DYNAMIC_STORAGE_BIT));
    free(initialParticles);
    
    __glcheck( gl()->BindBufferBase(GL_UNIFORM_BUFFER, 4, m_ubocompute) ); /* m_ubocompute (UBO) serves as the simulation parameters structure */
    m_computeSim.UniformBlock("SimulationConstants", 4);
    __glcheck( gl()->BindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_ssbodt) ); /* m_ssbodt (SSBO) serves as a structure to keep track that dt is in a valid range */
    m_computeSim.StorageBlock("ClampDeltaTime", 5);

    __glcheck( gl()->BindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_ssboparticle)); /* m_ssboparticle (SSBO) serves as the buffer of particles that will be shown on screen */
    m_computeVisual.StorageBlock("ParticlesForVisualization", 6);
    __glcheck( gl()->BindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_ssbodt));
    m_computeVisual.StorageBlock("ClampDeltaTime", 7);
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
    math::vec2f maxVel;
    math::vec2u winSize{acontext::windowSize(state.awc_context_id)};

    renderImGui(state);

    /* Update Graphics State - Swap Textures for I/O */
    std::swap(gfx.m_fluidtex[0], gfx.m_fluidtex[1]);
    // markfmt("std::swap() => { m_fluidtex[0]: %u | m_fluidtex[1]: %u }", gfx.m_fluidtex[0], gfx.m_fluidtex[1]);
    gfx.m_computeSim.bind();
    gfx.m_computeSim.uniform1i("infield", 0);
    __glcheck( gl()->BindTextureUnit(0, gfx.m_fluidtex[0]));
    __glcheck( gl()->BindImageTexture(1, gfx.m_fluidtex[1], 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F));
    __glcheck( gl()->DispatchCompute(sim_state.dims.x, sim_state.dims.y, 1));
    __glcheck( gl()->MemoryBarrier(GL_ALL_BARRIER_BITS));

    gfx.m_computeVisual.bind();
    __glcheck( gl()->BindImageTexture(2, gfx.m_fluidtex[1], 0, false, 0, GL_READ_WRITE, GL_RGBA32F));
    __glcheck( gl()->BindImageTexture(3, gfx.m_fluidtex[2], 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F));
    __glcheck( gl()->DispatchCompute(1, 1, 1));
    __glcheck( gl()->MemoryBarrier(GL_ALL_BARRIER_BITS));


    /* Draw Call */
    // __glcheck( gl()->BindFramebuffer(GL_READ_FRAMEBUFFER, gfx.m_fboid));
    // __glcheck( gl()->BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
    // __glcheck( gl()->BlitFramebuffer(0, 0, sim_state.dims.x, sim_state.dims.y, 0, 0, winSize[0], winSize[1], GL_COLOR_BUFFER_BIT, GL_LINEAR));
    __glcheck( gl()->BlitNamedFramebuffer(gfx.m_fboid, 0, 
        0, 0, sim_state.dims.x, sim_state.dims.y, 
        0, 0, winSize[0], winSize[1],
        GL_COLOR_BUFFER_BIT, 
        GL_LINEAR
    ));
    return;
}


void update(__unused ProgramState& state) {
    return;
}


} // namespace ProgramRender