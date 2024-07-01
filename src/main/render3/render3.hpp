#pragma once
#include "awc/awc.hpp"
#include "awc/usereventdef.hpp"
#include "util/allocator.hpp"
#include "util/vec.hpp"
#include "util/time.hpp"
#include "gl/shader2.hpp"
#include <ImGui/imgui.h>


namespace ainput = AWC::Input;
namespace acontext = AWC::Context;


int render3();


namespace Prog 
{


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
    u32 m_ssboparticle;
    bool m_refreshComputeSim{false};
    bool m_refreshComputeVisual{false};
    u8   reserved[10];
    ShaderProgramV2 m_computeSim;
    ShaderProgramV2 m_computeVisual;


    void prepare(
        math::vec2i const& sim_bounds
    );
    void destroy();
};


struct ProgramState
{
    glState          graphics;
    frameTimeData    timing;
    math::vec2i      sim_dims{256, 256};
    u32              code_block_counter{0};
    u8               awc_context_id;
    u8               reserved[3];
};

u8   init_awc();
void renderImGui(ProgramState& glob_state);
void render(ProgramState& glob_state);
void update(ProgramState& glob_state);


} // namespace Prog




inline void custom_mousebutton_callback(user_mousebutton_struct const* data)
{
    u8 state = (ainput::inputState::PRESS == data->action && data->button == ainput::mouseButton::RIGHT);
    if(state)
        ainput::unrestrictCursor();
    else
        ainput::unlockCursor();

    return;
}

inline u8 Prog::init_awc()
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


inline void Prog::renderImGui(ProgramState& state)
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




class Timestamp
{
private:
    std::array<Time::timepoint_nano, 2> last_copy;
    std::array<Time::timepoint_nano, 2> stamps;

    friend struct TimerAllocator;
public:
    void begin()
    {
        last_copy = stamps;
        stamps[0] = Time::now();
        return;
    }
    void end() {
        stamps[1] = Time::now();
        return;
    }


    i64 value() const {
        return (last_copy[1] - last_copy[0]).count();
    }

    template<typename T> T value_units(f64 HowManyUnitsIn1Second) const
    {
        f64 unitConvert = 1e-9 * HowManyUnitsIn1Second;
        return __scast(T, value() * unitConvert);
    }
};


struct TimerAllocator
{
    StaticPoolAllocator<Timestamp> m_buffer;


    void create(u32 finalSize)
    {
        m_buffer.create(finalSize);
        return;
    }
    void destroy()
    {
        m_buffer.destroy();
        return;
    }


    Timestamp* allocate() { return m_buffer.allocate(); };
    void   free(Timestamp* timer) { m_buffer.free(timer); }
};

