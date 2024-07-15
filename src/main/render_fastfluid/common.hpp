#ifndef __FASTFLUID_COMMON_DEFINTIONS_HEADER__
#define __FASTFLUID_COMMON_DEFINTIONS_HEADER__
#include "util/vec2.hpp"
#include "util/time.hpp"
#include "gl/shader2.hpp"


namespace Fluid 
{


using namespace util::math;


struct ParticleData {
    vec4f position;
    vec4f color;
};


typedef struct compute_shader_particle_buffer_definition
{
    u32          particleCount;
    u32          reserved[7];
    ParticleData buffer[1];
} ParticleBuffer;


class ParticleBufferManager
{
public:
    void create(u32 particleAmount)
    {
        const size_t bufferSize = sizeof(ParticleBuffer) + 
            sizeof(ParticleData) * ( particleAmount - 1);

        mem = __rcast(ParticleBuffer*, malloc(bufferSize));
        mem->particleCount = particleAmount;
    }
    
    void destroyCpuSide() {
        free(mem);
        return;
    }


    ParticleData& operator[](u32 idx) {
        // ifcrashstr_debug(idx >= mem->particleCount, "ParticleBufferManager::operator[](...) => Out-of-bounds Memory Access");
        return mem->buffer[idx];
    }

    size_t size()        const { return mem->particleCount; }
    size_t bytes()       const { return size() * sizeof(ParticleData); }
    size_t bytes_alloc() const { 
        return sizeof(ParticleBuffer) + 
        sizeof(ParticleData) * ( mem->particleCount - 1); 
    }
    ParticleBuffer const* data() const { return mem; }
private:
    ParticleBuffer* mem;
};


typedef struct __measuring_program_performance
{
    Time::Timestamp m_stamps[5];
    f64 m_interpolate_frame{0};
    u32 m_frameCount{0};
    u32 reserved{0};
    Time::Timestamp& measureLag = m_stamps[0];
    Time::Timestamp& frame  = m_stamps[1];
    Time::Timestamp& game   = m_stamps[2];
    Time::Timestamp& update = m_stamps[3];
    Time::Timestamp& render = m_stamps[4];
} frameTimeData;


struct glState {
    u32 m_fluidtex[4];
    u32 m_fboid;
    u32 m_ssboparticle;
    u8   reserved[10];
    bool m_refreshComputeSim{false};
    bool m_refreshComputeVisual{false};
    ShaderProgramV2 m_computeSim;
    ShaderProgramV2 m_computeVisual;
    std::vector<vec4f>    m_simInitialFields;
    std::vector<vec4f>    m_simUserInputForces;
    ParticleBufferManager m_simUserInputDye;


    static void initOpenGLState(glState& glstate, vec2i const& sim_bounds);
    static void destroyOpenGLState(glState& glstate);
};


struct ProgramState
{
    glState       graphics;
    frameTimeData timing;
    vec2i         sim_dims;
    u8            awc_context_id;
    bool          shouldRestartSimulation;
    bool          shouldRestartUserInputTexture;
    u8            reserved[5];
};


u8   init_awc();
void renderImGui(ProgramState& glob_state);
void render(ProgramState& glob_state);


} // namespace Prog


#endif