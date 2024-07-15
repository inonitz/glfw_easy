#include <glad/gl.h>
#include "render3.hpp"
#include "awc/opengl.hpp"
#include "util/marker.hpp"
#include "util/random.hpp"
#include "util/time.hpp"
#include <thread>
#include <filesystem>


i32 render3()
{
    Prog::ProgramState globalState;
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


    globalState.awc_context_id = Prog::init_awc();
    globalState.graphics.prepare(globalState.sim_dims);
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
        paused ^= ainput::isKeyPressed(ainput::keyCode::P);
        globalState.graphics.m_refreshComputeSim    ^= ainput::isKeyPressed(ainput::keyCode::NUM1);
        globalState.graphics.m_refreshComputeVisual ^= ainput::isKeyPressed(ainput::keyCode::NUM2);


        if(!paused) 
        {
            /* Game State Update */
            lastgame = gametime;
            gametime[0] = Time::now();
            while(lag >= ns_per_update) {
                updatetime[0] = Time::now();
                Prog::update(globalState);
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
            Prog::render(globalState);
            rendertime[1] = Time::now();
        } else {
            std::this_thread::sleep_for(ns_per_update);
        }


        /* Library State */
        AWC::end_frame();
        ++frameCounter;
        frametime[1] = Time::now();
    }

    globalState.graphics.destroy();
    AWC::destroy();
    return 0;
}




void Prog::glState::prepare(math::vec2i const& sim_bounds) 
{
    static constexpr const char* shaderName[2] = { "dead_simple.comp", "visual.comp" };
    static const std::string shaderPath[2] = {
        ( std::filesystem::current_path()/std::filesystem::path{std::string{"src/main/render3/"}}/std::filesystem::path{shaderName[0]} ).generic_u8string(),
        ( std::filesystem::current_path()/std::filesystem::path{std::string{"src/main/render3/"}}/std::filesystem::path{shaderName[1]} ).generic_u8string()
    };

    /* First-Time Memory Init */
    /* Fill Texture with 0'th iteration data */
    std::vector<math::vec4f> initialData{__scast(u64, sim_bounds.x * sim_bounds.y)};
    for(auto& p : initialData) {
        p = { random32f(), random32f(), 1.0f, 0.0f };
    }
    /* Fill Particle Buffer with Positions & Dyes */
    const u32 particleCount = sim_bounds.x * sim_bounds.y;
    const size_t bufferSize = sizeof(ParticleBuffer) + sizeof(ParticleBuffer::ParticleData) * ( particleCount - 1);
    ParticleBuffer* initialParticles = __rcast(ParticleBuffer*, malloc(bufferSize));
    initialParticles->particleCount = particleCount;
    for(i32 i = 0; i < sim_bounds.x; ++i) 
    {
        for(i32 j = 0; j < sim_bounds.y; ++j) {
            initialParticles->buffer[i * sim_bounds.x + j].position = { random32f() * sim_bounds.x, random32f() * sim_bounds.y, 0.0f, 0.0f };
            // initialParticles->buffer[i * sim_bounds.x + j].position = { __scast(f32, i), __scast(f32, j), 0.0f, 0.0f };
            initialParticles->buffer[i * sim_bounds.x + j].color = { 1.0f, 1.0f, 1.0f, 1.0f };
        }
    }


    m_computeSim.createFrom({ { shaderPath[0].data(), GL_COMPUTE_SHADER } });
    m_computeSim.resizeLocalWorkGroup(0, { 1, 1, 1 });
    m_computeVisual.createFrom({ { shaderPath[1].data(), GL_COMPUTE_SHADER } });
    m_computeVisual.resizeLocalWorkGroup(0, { 1, 1, 1 });
    __release_unused bool status = m_computeSim.compile();
    ifcrash_debug(!status);
    status = m_computeVisual.compile();
    ifcrash_debug(!status);


    gl()->CreateTextures(GL_TEXTURE_2D, 3, m_fluidtex);
    gl()->CreateBuffers(1, &m_ssboparticle);
    gl()->CreateFramebuffers(1, &m_fboid);


    /* Buffer Configuration & Allocation */
    /* fluidtex[0] & fluidtex[1] are constantly swapped and serve as I/O for the compute shader */
    gl()->TextureParameteri(m_fluidtex[0], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl()->TextureParameteri(m_fluidtex[0], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl()->TextureParameteri(m_fluidtex[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl()->TextureParameteri(m_fluidtex[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl()->TextureStorage2D(m_fluidtex[0], 1, GL_RGBA32F, sim_bounds.x, sim_bounds.y);
    gl()->TextureParameteri(m_fluidtex[1], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl()->TextureParameteri(m_fluidtex[1], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl()->TextureParameteri(m_fluidtex[1], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl()->TextureParameteri(m_fluidtex[1], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl()->TextureStorage2D(m_fluidtex[1], 1, GL_RGBA32F, sim_bounds.x, sim_bounds.y);
    /* fluidtex[2] serves as the visual representation of the velocity field - the actual drawing part */
    gl()->TextureParameteri(m_fluidtex[2], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl()->TextureParameteri(m_fluidtex[2], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl()->TextureParameteri(m_fluidtex[2], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl()->TextureParameteri(m_fluidtex[2], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl()->TextureStorage2D(m_fluidtex[2], 1, GL_RGBA32F, sim_bounds.x, sim_bounds.y);


    /* FBO Config & Image-Bind to outpos */
    gl()->NamedFramebufferTexture(m_fboid, GL_COLOR_ATTACHMENT0, m_fluidtex[2], 0);
    u32 fbstatus;
    while(  ( fbstatus = gl()->CheckNamedFramebufferStatus(m_fboid, GL_FRAMEBUFFER) ) != GL_FRAMEBUFFER_COMPLETE  ) {}


    gl()->TextureSubImage2D(m_fluidtex[0], 0, 0, 0, sim_bounds.x, sim_bounds.y, GL_RGBA, GL_FLOAT, initialData.data());
    gl()->NamedBufferStorage(m_ssboparticle, bufferSize, initialParticles, GL_DYNAMIC_STORAGE_BIT);
    free(initialParticles);
    
    gl()->BindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_ssboparticle); /* m_ssboparticle (SSBO) serves as the buffer of particles that will be shown on screen */
    m_computeVisual.StorageBlock("ParticlesForVisualization", 4);
    return;
}


void Prog::glState::destroy()
{
    gl()->DeleteTextures(3, m_fluidtex);
    gl()->DeleteBuffers(1, &m_ssboparticle);
    gl()->DeleteFramebuffers(1, &m_fboid);
    m_computeSim.destroy();
    m_computeVisual.destroy();
    return;
}


void Prog::render(ProgramState& state) {
    auto& gfx = state.graphics;
    math::vec2u winSize{acontext::windowSize(state.awc_context_id)};
    u8 recompiling{0};
    u8 status{1};


    if(gfx.m_refreshComputeSim) {
        gfx.m_computeSim.refreshFromFiles();
        gfx.m_computeSim.resizeLocalWorkGroup(0, { 1, 1, 1 });
        recompiling = 1;
        status = recompiling * gfx.m_computeSim.compile();
        gfx.m_refreshComputeSim = !(status == 1);
    }
    if(gfx.m_refreshComputeVisual) {
        gfx.m_computeVisual.refreshFromFiles();
        gfx.m_computeVisual.resizeLocalWorkGroup(0, { 1, 1, 1 });
        recompiling = 2;
        status = recompiling * gfx.m_computeVisual.compile();
        gfx.m_refreshComputeVisual = !(status == 2);
    }
    if(recompiling && status == 0) {
        return;
    }


    renderImGui(state);
    /* Update Graphics State - Swap Textures for I/O */
    std::swap(gfx.m_fluidtex[0], gfx.m_fluidtex[1]);
    // markfmt("std::swap() => { m_fluidtex[0]: %u | m_fluidtex[1]: %u }", gfx.m_fluidtex[0], gfx.m_fluidtex[1]);
    gfx.m_computeSim.bind();
    gfx.m_computeSim.uniform1i("infield", 0);
    gl()->BindTextureUnit(0, gfx.m_fluidtex[0]);
    gl()->BindImageTexture(1, gfx.m_fluidtex[1], 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    gl()->DispatchCompute(state.sim_dims.x, state.sim_dims.y, 1);
    gl()->MemoryBarrier(GL_ALL_BARRIER_BITS);

    gfx.m_computeVisual.bind();
    gfx.m_computeVisual.uniform1i("outfield", 2);
    gfx.m_computeVisual.uniform1f("frameCounter", state.timing.m_frameCount);
    gl()->BindTextureUnit(2, gfx.m_fluidtex[1]);
    gl()->BindImageTexture(3, gfx.m_fluidtex[2], 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    gl()->DispatchCompute(state.sim_dims.x, state.sim_dims.y, 1);
    gl()->MemoryBarrier(GL_ALL_BARRIER_BITS);


    /* Draw Call */
    gl()->BlitNamedFramebuffer(gfx.m_fboid, 0, 
        0, 0, state.sim_dims.x, state.sim_dims.y, 
        0, 0, winSize[0], winSize[1],
        GL_COLOR_BUFFER_BIT, 
        GL_LINEAR
    );
    return;
}


void Prog::update(__unused ProgramState& state) {
    return;
}
