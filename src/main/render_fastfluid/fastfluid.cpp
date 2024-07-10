#include "fastfluid.hpp"
#include "common.hpp"
#include "awc/awc.hpp"
#include "awc/opengl.hpp"
#include <thread>
#include <glbinding/gl46core/gl.h>


namespace AWCIN = AWC::Input;


i32 render_fastfluid()
{
    Encapsulate::ProgramState globalState;
    Encapsulate::frameTimeData& time = globalState.timing;
    bool alive{true}, paused{false};
    constexpr u32 minFrameSkips = 8;
    constexpr u32 targetFrameRate{144};
    constexpr f64 ns_per_frame = 1e+9f / targetFrameRate;
    const Time::nanosecond ns_per_update{__scast(i64, ns_per_frame)};
    Time::nanosecond lag{0};


    globalState.awc_context_id = Encapsulate::init_awc();
    globalState.sim_dims = util::math::vec2i{1024, (1024 / 16) * 9 };
    Encapsulate::glState::initOpenGLState(globalState.graphics, globalState.sim_dims);
    std::swap(globalState.graphics.m_fluidtex[0], globalState.graphics.m_fluidtex[2]); /* Swap Texture ID's for I/O */


    time.measureLag.begin();
    while(alive) {
    TIME_NAMESPACE_TIME_CODE_BLOCK(time.frame, 
    {
        time.measureLag.end();
        lag += time.measureLag.curr_value();
        AWC::begin_frame();


        alive  = AWC::Context::windowActive(globalState.awc_context_id);
        alive  &= !AWCIN::isKeyPressed(AWCIN::keyCode::ESCAPE);
        paused ^= AWCIN::isKeyPressed(AWCIN::keyCode::P);
        globalState.graphics.m_refreshComputeSim    ^= AWCIN::isKeyPressed(AWCIN::keyCode::NUM1);
        globalState.graphics.m_refreshComputeVisual ^= AWCIN::isKeyPressed(AWCIN::keyCode::NUM2);
        
        if(unlikely(time.m_frameCount < minFrameSkips) || paused) {
            std::this_thread::sleep_for(ns_per_update);
        } else 
        {
            TIME_NAMESPACE_TIME_CODE_BLOCK(time.render, {
                time.m_interpolate_frame = __scast(f64, time.frame.value_units<f64>(1e+9)) / ns_per_frame;
                Encapsulate::render(globalState);
            });
        }


        AWC::end_frame();
        ++time.m_frameCount;
    });
    }


    Encapsulate::glState::destroyOpenGLState(globalState.graphics);
    AWC::destroy();
    return 0;
}


void Encapsulate::render(ProgramState& state) {
    auto& gfx = state.graphics;
    const util::math::vec2u winSize{AWC::Context::windowSize(state.awc_context_id)};
    const util::math::vec4f rgba{0.0f, 0.5f, 0.7f, 1.0f};
    // const util::math::vec4f rgba{0.0f};
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
    /* Clear Screen */
    gl()->ClearNamedFramebufferfv(gfx.m_fboid, GL_COLOR, 0, rgba.begin());


    std::swap(gfx.m_fluidtex[0], gfx.m_fluidtex[2]); /* Swap Texture ID's for I/O */
    gfx.m_computeSim.bind();
    gfx.m_computeSim.uniform1i("infield", 0);
    gl()->BindTextureUnit(0, gfx.m_fluidtex[0]);
    gl()->BindTextureUnit(1, gfx.m_fluidtex[1]);
    gl()->BindImageTexture(2, gfx.m_fluidtex[2], 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    gl()->MemoryBarrier(GL_ALL_BARRIER_BITS);
    gl()->DispatchCompute(state.sim_dims.x, state.sim_dims.y, 1);

    gfx.m_computeVisual.bind();
    gfx.m_computeVisual.uniform1i("outfield", 2);
    gl()->BindTextureUnit(3, gfx.m_fluidtex[2]);
    gl()->BindImageTexture(4, gfx.m_fluidtex[3], 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
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
