#include "common.hpp"
#include "gl/shader2.hpp"
#include "util/random.hpp"
#include "awc/awc.hpp"
#include "awc/usereventdef.hpp"
#include <glbinding/gl/gl.h>
#include <ImGui/imgui.h>
#include <filesystem>
#include "util/marker2.hpp"


namespace AWCIN = AWC::Input;
using namespace util::math;
using namespace gl;


void Fluid::glState::initOpenGLState(glState& glstate, vec2i const& sim_bounds)
{
    auto& gfx = glstate;
    static constexpr const char* shaderName[2] = { "rewrite2.comp", "visual.comp" };
    static const std::string shaderPath[2] = {
        ( std::filesystem::current_path()/std::filesystem::path{std::string{"src/main/render_fastfluid/"}}/std::filesystem::path{shaderName[0]} ).generic_u8string(),
        ( std::filesystem::current_path()/std::filesystem::path{std::string{"src/main/render_fastfluid/"}}/std::filesystem::path{shaderName[1]} ).generic_u8string()
    };


    /* First-Time Memory Init */
    /* Fill Texture with 0'th iteration data */
    gfx.m_simInitialFields.resize(sim_bounds.x * sim_bounds.y);
    for(auto& p : gfx.m_simInitialFields) {
        p = { random32f(), random32f(), 1.0f, 0.0f };
    }

    /* Fill Particle Buffer with Positions & Dyes */
    gfx.m_simUserInputDye.create(sim_bounds.x * sim_bounds.y);
    vec4f color(0.0f, 0.5f, 0.7f, 1.0f);
    const vec2f sim_bounds_inv{
        1.0f / __scast(f32, sim_bounds.x),
        1.0f / __scast(f32, sim_bounds.y)
    };
    for(i32 j = 0; j < sim_bounds.y; ++j) {
        for(i32 i = 0; i < sim_bounds.x; ++i)
        {
            static vec4f pos = { __scast(f32, i), __scast(f32, j), 0.0f, 0.0f };
            color.x = pos.x * sim_bounds_inv.x;
            color.y = pos.y * sim_bounds_inv.y;
            gfx.m_simUserInputDye[j * sim_bounds.x + i].position = pos;
            gfx.m_simUserInputDye[j * sim_bounds.x + i].color = color;
        }
    }


    /* Create OpenGL objects */
    gfx.m_computeSim.createFrom({
        ShaderData{ shaderPath[0].data(), __scast(u32, GL_COMPUTE_SHADER) }
    });
    gfx.m_computeSim.resizeLocalWorkGroup(0, { 1, 1, 1 });
    gfx.m_computeVisual.createFrom({
        ShaderData{ shaderPath[1].data(), __scast(u32, GL_COMPUTE_SHADER) }
    });
    gfx.m_computeVisual.resizeLocalWorkGroup(0, { 1, 1, 1 });
    __release_unused bool status = gfx.m_computeSim.compile();
    ifcrash_debug(!status);
    status = gfx.m_computeVisual.compile();
    ifcrash_debug(!status);


    glCreateTextures(GL_TEXTURE_2D, 4, gfx.m_fluidtex);
    glCreateBuffers(1, &gfx.m_ssboparticle);
    glCreateFramebuffers(1, &gfx.m_fboid);
    /* Buffer Configuration & Allocation */
    /* fluidtex[0] & fluidtex[2] are constantly swapped and serve as I/O for the compute shader */
    glTextureParameteri(gfx.m_fluidtex[0], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(gfx.m_fluidtex[0], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(gfx.m_fluidtex[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(gfx.m_fluidtex[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureStorage2D(gfx.m_fluidtex[0], 1, GL_RGBA32F, sim_bounds.x, sim_bounds.y);
    /* fluidtex[1] is for user input & interaction - dye, external forces, etc */
    glTextureParameteri(gfx.m_fluidtex[1], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(gfx.m_fluidtex[1], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(gfx.m_fluidtex[1], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(gfx.m_fluidtex[1], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureStorage2D(gfx.m_fluidtex[1], 1, GL_RGBA32F, sim_bounds.x, sim_bounds.y);
    glTextureParameteri(gfx.m_fluidtex[2], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(gfx.m_fluidtex[2], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(gfx.m_fluidtex[2], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(gfx.m_fluidtex[2], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureStorage2D(gfx.m_fluidtex[2], 1, GL_RGBA32F, sim_bounds.x, sim_bounds.y);
    /* fluidtex[3] serves as the visual representation of the velocity field - the actual drawing part */
    glTextureParameteri(gfx.m_fluidtex[3], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(gfx.m_fluidtex[3], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(gfx.m_fluidtex[3], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(gfx.m_fluidtex[3], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureStorage2D(gfx.m_fluidtex[3], 1, GL_RGBA32F, sim_bounds.x, sim_bounds.y);


    /* FBO Config & Image-Bind to outpos */
    glNamedFramebufferTexture(gfx.m_fboid, GL_COLOR_ATTACHMENT0, gfx.m_fluidtex[3], 0);
    GLenum fbstatus;
    while(  ( fbstatus = glCheckNamedFramebufferStatus(gfx.m_fboid, GL_FRAMEBUFFER) ) != GL_FRAMEBUFFER_COMPLETE  ) {}


    glTextureSubImage2D(gfx.m_fluidtex[0], 0, 0, 0, sim_bounds.x, sim_bounds.y, GL_RGBA, GL_FLOAT, gfx.m_simInitialFields.data());
    glNamedBufferStorage(gfx.m_ssboparticle, gfx.m_simUserInputDye.bytes_alloc(), gfx.m_simUserInputDye.data(), GL_DYNAMIC_STORAGE_BIT);
    /* m_ssboparticle (SSBO) serves as the buffer of particles that will be shown on screen */
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, gfx.m_ssboparticle);
    gfx.m_computeVisual.StorageBlock("ParticlesForVisualization", 5);
    return;
}


void Fluid::glState::destroyOpenGLState(glState &glstate)
{
    glDeleteTextures(3, glstate.m_fluidtex);
    glDeleteBuffers(1, &glstate.m_ssboparticle);
    glDeleteFramebuffers(1, &glstate.m_fboid);
    glstate.m_computeSim.destroy();
    glstate.m_computeVisual.destroy();
    glstate.m_simInitialFields.resize(0);
    glstate.m_simUserInputForces.resize(0);
    glstate.m_simUserInputDye.destroyCpuSide();
    return;
}




inline void custom_mousebutton_callback(user_mousebutton_struct const* data)
{
    u8 state = (AWCIN::inputState::PRESS == data->action && data->button == AWCIN::mouseButton::RIGHT);
    if(state)
        AWCIN::unrestrictCursor();
    else
        AWCIN::unlockCursor();

    return;
}

u8 Fluid::init_awc()
{
    u8 ctxid;
    
    AWC::init();
    ifcrash( (ctxid = AWC::Context::allocate() ) == 0);
    
    AWC::Context::setActive(ctxid);
    AWC::Context::init(ctxid,
        AWC::WindowOptions{{{
            WINDOW_OPTION_FRAMEBUFFER_BITS_DEFAULT, 
            WINDOW_OPTION_DEFAULT | WINDOW_OPTION_RESIZABLE, 
            144, 
            0 
        }}},
        AWC::WindowDescriptor{ {{ 1280, 720 }}, nullptr }
    );
    AWC::Event::setUserCallback(&custom_mousebutton_callback);
    return ctxid;
}


void Fluid::renderImGui(ProgramState& state)
{
    f64 frameTimeDouble  = state.timing.frame .value_units<f64>(1e+3);
    f64 gameTimeDouble   = state.timing.game  .value_units<f64>(1e+3);
    f64 renderTimeDouble = state.timing.render.value_units<f64>(1e+3);
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