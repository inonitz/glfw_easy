#include "render0.hpp"
#include "awc/inputdef.hpp"
#include "common.hpp"
#include <ImGui/imgui.h>
#include "util/random.hpp"
#include "util/time.hpp"
#include "awc/awc.hpp"
#include "awc/opengl.hpp"
#include "gl/shader2.hpp"


namespace ainput = AWC::Input;
namespace acontext = AWC::Context;


u8 init_awc()
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


inline constexpr void fill_particle_buffer(ParticleBuffer& buf)
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
    u32 vao_id;
    u32 particleDataGPU[2];
    ColorBuffer    particleColors;
    ParticleBuffer drawBuffer;
    math::mat4f    identity;
    f32            k_pradius{5.0f};

    bool updateDrawBuffer;
};


void render(
    u32 frameCount, 
    f32 frameTime, 
    f32 gameTime, 
    f32 renderTime, 
    f64 lerp,
    glState& gstate
);
void update(glState& state);




i32 render0()
{
    auto ctxtid = init_awc();
    bool alive{true};
    Time::timepoint_nano prev, curr;
    std::array<Time::timepoint_nano, 2> 
        frametime, 
        rendertime, 
        gametime, 
        updatetime, 
        lastframe, 
        lastrender, 
        lastgame;
    u32 frameCounter{0}; 
    constexpr u32 particleAmount = 2048;
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
    glState state;


    state.vertfrag.createFrom({
        { "misc/shaders/default/bshader.vert", GL_VERTEX_SHADER   },
        { "misc/shaders/default/bshader.frag", GL_FRAGMENT_SHADER }
    });
    __unused auto status = state.vertfrag.compile();
    ifcrash(!status);


    state.particleColors.resize(particleAmount);
    state.drawBuffer.resize(particleAmount);
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



    state.vertfrag.bind();
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
            update(state);
            updatetime[1] = Time::now();
            lag -= (ns_per_update - (updatetime[1] - updatetime[0]) );
        }
        gametime[1] = Time::now();


        /* Render State Update */
        lastrender = rendertime;
        rendertime[0] = Time::now();
        render( 
            frameCounter,
            Time::dursecondf32{lastframe[1] - lastframe[0]}.count(), 
            Time::dursecondf32{lastgame[1] - lastgame[0]}.count(), 
            Time::dursecondf32{lastrender[1] - lastrender[0]}.count(), 
            __scast(f64, lag.count()) / ns_per_frame,
            state
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


void render(
    u32 frameCount, 
    f32 frameTime, 
    f32 gameTime, 
    f32 renderTime, 
    f64 lerp,
    glState& gstate
) {
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
Frame Time 		     %3.5f [ms]\n \
  Game   State %3.5f [ms] (%2.2f%%)\n \
  Render State %3.5f [ms] (%2.2f%%)\n \
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


    if(gstate.updateDrawBuffer) {
        fill_particle_buffer(gstate.drawBuffer);
        transform_to_screen_space(gstate.drawBuffer);
        refresh_gpu_data(gstate.particleDataGPU[0], gstate.drawBuffer);
        gstate.updateDrawBuffer = false;
    }


    gstate.vertfrag.uniformMatrix4fv("modelmatrix", gstate.identity.data());
    gstate.vertfrag.uniform1f("particleSize", gstate.k_pradius);
    /* Draw Call */
    __glcheck( gl()->DrawArrays(GL_POINTS, 0, gstate.drawBuffer.size()); );
    return;
}


void update(glState& state) {
    state.updateDrawBuffer = ainput::isKeyPressed(ainput::keyCode::R) || ainput::isKeyRepeated(ainput::keyCode::R);
    state.k_pradius += ainput::isKeyPressed(ainput::keyCode::NUM1) || ainput::isKeyRepeated(ainput::keyCode::NUM1);
    state.k_pradius -= ainput::isKeyPressed(ainput::keyCode::NUM2) || ainput::isKeyRepeated(ainput::keyCode::NUM2);
    return;
}