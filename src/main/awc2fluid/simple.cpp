#include "simple.hpp"
#include "awc2/include/awc2.hpp"
#include "glbinding/gl/functions.h"
#include "util/random.hpp"
#include "util/marker2.hpp"
#include <algorithm>
#include <chrono>
#include <thread>
#include <filesystem>
#include <glbinding/gl/gl.h>
#include <ImGui/imgui.h>


using namespace util::math;
namespace AIN = AWC2::Input;
namespace fs = std::filesystem;


void GenericNamespaceName::ParticleBufferManager::set()
{
    /* Set Default Values */
    static vec4f color{0.0f};
    static vec4f pos{0.0f};
    const vec2f dimensions_inv {
        1.0f / __scast(f32, mem->m_width),
        1.0f / __scast(f32, mem->m_height)
    };


    std::printf("Begin ParticleBufferManager print(%ux%u) => {", mem->m_width, mem->m_height);
    for(u32 i = 0; i < mem->m_width; ++i) {
        for(u32 j = 0; j < mem->m_height; ++j) 
        {
            pos = { __scast(f32, i), __scast(f32, j), 0.0f, 0.0f };
            color.x = pos.x * dimensions_inv.x;
            color.y = pos.y * dimensions_inv.y;
            mem->buffer[i * mem->m_height + j].position = pos;
            mem->buffer[i * mem->m_height + j].color    = color;
            // auto* colorstr = color.to_string();
            // auto* posstr   = pos.to_string();
            // std::printf("\n%s\n%s\n", colorstr, posstr);
            // std::free(colorstr);
            // std::free(posstr);
        }
    }
    std::printf("} End\n");
    return;
}



inline void render(
    GenericNamespaceName::OpenGLDataV2& gfxdata,
    GenericNamespaceName::frameTimeData& framedata,
    vec2i const& simDims
);




inline void custom_mousebutton_callback(AWC2::user_callback_mousebutton_struct const* data)
{
    u8 state = (
        AIN::mouseButton::RIGHT == data->button &&
        AIN::inputState::PRESS  == data->action
    );
    if(state)
        AIN::setCursorMode(AIN::cursorMode::SCREEN_BOUND);
    else
        AIN::setCursorMode(AIN::cursorMode::NORMAL);


    return;
}


i32 render_fluid_awc2_fuckyou()
{
    bool alive{true}, paused{false};
    constexpr bool slowRender{false};
    AWC2::ContextID   g_awc2id;
    __unused bool     g_shouldRestartSimulation;
    __unused bool     g_shouldRestartUserInputTexture;
    util::math::vec2i g_simDims;
    __unused GenericNamespaceName::frameTimeData g_timing;
    GenericNamespaceName::OpenGLDataV2 gfx;


    /* Init AWC2 */
    {
        markstr("AWC2 init begin");
        AWC2::init();
        g_awc2id = AWC2::createContext();
        g_simDims = { 2560, 1440 };
        AWC2::initializeContext(g_awc2id, 2560, 1440, AWC2::WindowDescriptor{});
        AWC2::setContextUserCallback(g_awc2id, &custom_mousebutton_callback);
        markstr("AWC2 init end");
    }
    /* OpenGL Data Initialization */
    {
        markstr("Opengl Data init begin");
        static constexpr const char* shaderName[2] = { "old_sim.comp", "old_visual.comp" };
        static constexpr const char* dirName       = "src/main/awc2fluid/";
        static const std::string shaderPath[2] = {
            ( fs::current_path()/fs::path{dirName}/fs::path{shaderName[0]} ).generic_u8string(),
            ( fs::current_path()/fs::path{dirName}/fs::path{shaderName[1]} ).generic_u8string()
        };

        gfx.m_simulation.createFrom({
            ShaderData{ shaderPath[0].data(), __scast(u32, gl::GL_COMPUTE_SHADER) }
        });
        gfx.m_simulation.resizeLocalWorkGroup(0, { 1, 1, 1 });
        gfx.m_screenOutput.createFrom({
            ShaderData{ shaderPath[1].data(), __scast(u32, gl::GL_COMPUTE_SHADER) }
        });
        gfx.m_screenOutput.resizeLocalWorkGroup(0, { 1, 1, 1 });
        ifcrash(!gfx.m_simulation.compile());
        ifcrash(!gfx.m_screenOutput.compile());
        

        /* Initialize Initial Simulation Buffers */
        gfx.m_inputDye.create(g_simDims);
        gfx.m_inputDye.set();
        gfx.m_inputForces.resize(g_simDims.x * g_simDims.y, vec4f{});
        gfx.m_initialFields.resize(g_simDims.x * g_simDims.y);
        std::generate(
            gfx.m_initialFields.begin(),
            gfx.m_initialFields.end(),
            []() -> vec4f {
                vec2f result = { random32f(), random32f() };
                return vec4f{ result.x, result.y, 1.0f, 0.0f };
            }
        );
        // markstr("test");
        // for(auto& val : gfx.m_initialFields) {
        //     auto* p = val.to_string();
        //     std::printf("%s\n", p);
        //     std::free(p);
        // }
        /*
            fluidtex[1] - Dye, External Forces, etc ... (User Interaction)
            fluidtex[0] - Constanstly Swapped with fluidtex[2], Velocity Frames of the simulation
            fluidtex[2] - Constanstly Swapped with fluidtex[0], Velocity Frames of the simulation
            fluidtex[3] - Screen Texture (Coupled with fboid)
            fboid        - Screen Framebuffer, to see simulation output
            ssboparticle - Shader Storage Buffer Object for updating particles' data
        */
        gl::glCreateTextures(gl::GL_TEXTURE_2D, 4, gfx.m_fluidtex);
        gl::glCreateFramebuffers(1, &gfx.m_fboid);
        gl::glCreateBuffers(1, &gfx.m_ssboparticle);
        for(auto const& tex : gfx.m_fluidtex) {
            gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
            gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
            gl::glTextureParameteri(tex, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR);
            gl::glTextureParameteri(tex, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
            gl::glTextureStorage2D(tex, 1, gl::GL_RGBA32F, g_simDims.x, g_simDims.y);
        }
        
        gl::glTextureSubImage2D(gfx.m_fluidtex[0], 0, 0, 0, 
            g_simDims.x, 
            g_simDims.y, 
            gl::GL_RGBA, 
            gl::GL_FLOAT, 
            gfx.m_initialFields.data()
        );

        gl::glNamedFramebufferTexture(gfx.m_fboid, gl::GL_COLOR_ATTACHMENT0, gfx.m_fluidtex[3], 0);
        ifcrash( gl::glCheckNamedFramebufferStatus(gfx.m_fboid, gl::GL_FRAMEBUFFER) 
            != gl::GL_FRAMEBUFFER_COMPLETE 
        );

        gl::glNamedBufferStorage(gfx.m_ssboparticle, 
            gfx.m_inputDye.bytes_alloc(), 
            gfx.m_inputDye.data(), 
            gl::GL_DYNAMIC_STORAGE_BIT
        );
        gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 5, gfx.m_ssboparticle);
        gfx.m_screenOutput.StorageBlock("ParticlesForVisualization", 5);


        std::swap(
            gfx.m_fluidtex[0], 
            gfx.m_fluidtex[2]
        ); /* Swap Texture ID's for I/O */
        markstr("Opengl Data init end");
    }


    /* Main App Loop */
    markstr("Main App Loop Begin");
    AWC2::setCurrentContext(g_awc2id);
    while(alive)
    {
        AWC2::newFrame();
        AWC2::begin();
        if(paused) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(6944444));
        } else {
            if constexpr (slowRender) {
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
            gfx.m_refreshComputeSim    ^= AWC2::Input::isKeyPressed(AWC2::Input::keyCode::NUM1);
            gfx.m_refreshComputeVisual ^= AWC2::Input::isKeyPressed(AWC2::Input::keyCode::NUM2);
            render(gfx, g_timing, g_simDims);
        }
        alive   = !AWC2::getContextStatus(g_awc2id) && !AWC2::Input::isKeyPressed(AWC2::Input::keyCode::ESCAPE);
        paused ^= AWC2::Input::isKeyPressed(AWC2::Input::keyCode::P);
        AWC2::end();
        ++g_timing.m_frameCount;
    }
    markstr("Main App Loop End");



    /* Destroy OpenGL Data */
    { 
        markstr("Destroy OpenGL data Begin");
        gl::glDeleteTextures(4, gfx.m_fluidtex);
        gl::glDeleteBuffers(1, &gfx.m_ssboparticle);
        gl::glDeleteFramebuffers(1, &gfx.m_fboid);
        gfx.m_simulation.destroy();
        gfx.m_screenOutput.destroy();
        gfx.m_initialFields.resize(0);
        gfx.m_inputForces.resize(0);
        gfx.m_inputDye.destroyCpuSide();
        markstr("Destroy OpenGL data End");
    }
    /* Destroy AWC2 Context & State */
    {
        markstr("Destroy AWC2 Context Begin");
        g_simDims = { DEFAULT32, DEFAULT32 };
        g_shouldRestartUserInputTexture = false;
        g_shouldRestartSimulation = false;
        AWC2::destroyContext(g_awc2id);
        AWC2::destroy();
        g_awc2id = DEFAULT8;
        markstr("Destroy AWC2 Context End");
    }
    return g_awc2id;
}




inline void render(
    GenericNamespaceName::OpenGLDataV2& gfxdata,
    GenericNamespaceName::frameTimeData& framedata,
    vec2i const& simDims
) {
    auto& gfx = gfxdata;
    auto winSize = AWC2::getCurrentContextViewport();
    const vec4f rgba{0.0f, 0.0f, 0.0f, 1.0f};
    u8 recompiling{0};
    u8 status{1};


    if(gfx.m_refreshComputeSim) {
        gfx.m_simulation.refreshFromFiles();
        gfx.m_simulation.resizeLocalWorkGroup(0, { 1, 1, 1 });
        recompiling = 1;
        status = recompiling * gfx.m_simulation.compile();
        gfx.m_refreshComputeSim = !(status == 1);
    }
    if(gfx.m_refreshComputeVisual) {
        gfx.m_screenOutput.refreshFromFiles();
        gfx.m_screenOutput.resizeLocalWorkGroup(0, { 1, 1, 1 });
        recompiling = 2;
        status = recompiling * gfx.m_screenOutput.compile();
        gfx.m_refreshComputeVisual = !(status == 2);
    }
    if(recompiling && status == 0) {
        return;
    }


    /* Render ImGui Window */
    {
        static f64 frameTimeDouble  = framedata.frame .value_units<f64>(1e+3);
        static f64 gameTimeDouble   = framedata.game  .value_units<f64>(1e+3);
        static f64 renderTimeDouble = framedata.render.value_units<f64>(1e+3);
        static f64 fps = 1e+3f / frameTimeDouble;


        ImGui::Begin("Program Statistics");
        ImGui::Text("\
Previous Frame Statistics:\n \
Frame Counter 		 %u\n \
Frame Time 		     %3.5f [ms]\n \
Game   State %3.5f [ms] (%2.2f%%)\n \
Render State %3.5f [ms] (%2.2f%%)\n \
Frames Per Second    %u\n \
Interpolation Factor %3.5f (now)\n",
            framedata.m_frameCount, 
            frameTimeDouble, 
            gameTimeDouble, 
            100.0f * (gameTimeDouble / frameTimeDouble), 
            renderTimeDouble, 
            100.0f * (renderTimeDouble / frameTimeDouble), 
            __scast(u32, fps),
            framedata.m_interpolate_frame
        );
        ImGui::End();
    }


    /* Clear Screen */
    gl::glClearNamedFramebufferfv(gfx.m_fboid, gl::GL_COLOR, 0, rgba.begin());
    std::swap(
        gfx.m_fluidtex[0], 
        gfx.m_fluidtex[2]
    ); /* Swap Texture ID's for I/O */
    gfx.m_simulation.bind();
    gfx.m_simulation.uniform1i("infield",        0);
    gfx.m_simulation.uniform1i("externalForces", 1);
    gfx.m_simulation.uniform1i("outfield",       2);
    gl::glBindTextureUnit(0, gfx.m_fluidtex[0]);
    gl::glBindTextureUnit(1, gfx.m_fluidtex[1]);
    gl::glBindImageTexture(2, gfx.m_fluidtex[2], 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    gl::glDispatchCompute(simDims.x, simDims.y, 1);


    /* outfield -> 3; 3 -> fluidtex[2] | outfield => fluidtex[2] | becomes the input */
    /* outpos   -> 4; 4 -> fluidtex[3] | outpos   => fluidtex[3] | output to screen  */
    gfx.m_screenOutput.bind();
    gfx.m_screenOutput.uniform1i("outfield", 3);
    gfx.m_screenOutput.uniform1i("outpos",   4);
    gl::glBindTextureUnit(3, gfx.m_fluidtex[2]);
    gl::glBindImageTexture(4, gfx.m_fluidtex[3], 0, false, 0, 
        gl::GL_WRITE_ONLY, 
        gl::GL_RGBA32F
    );
    gl::glDispatchCompute(simDims.x, simDims.y, 1);
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);


    /* Draw Call */
    gl::glBlitNamedFramebuffer(gfx.m_fboid, 0, 
        0, 0, simDims.x, simDims.y, 
        0, 0, winSize.x, winSize.y,
        gl::GL_COLOR_BUFFER_BIT, 
        gl::GL_LINEAR
    );
    return;
}