#pragma once
#include "awc2/include/context.hpp"
#include "util/aligned_malloc.hpp"
#include "util/vec2.hpp"
#include "util/time.hpp"
#include "gl/shader2.hpp"
#include <cstdio>


int render_fluid_awc2_fuckyou();


namespace GenericNamespaceName {


using namespace util::math;


struct ParticleData {
    vec4f position;
    vec4f color;
};


typedef struct __compute_shader_particle_buffer_definition
{
    u32          m_particleCount;
    u32          m_width;
    u32          m_height;
    u32          reserved[5];
    ParticleData buffer[1];
} ParticleBuffer;


class ParticleBufferManager
{
public:
    void create(vec2i const& dimensions)
    {
        const size_t bufferSize = sizeof(ParticleBuffer) + 
            sizeof(ParticleData) * ( dimensions.x * dimensions.y - 1);

        mem = __rcast(ParticleBuffer*, util::aligned_malloc<PAGE>(bufferSize));
        mem->m_particleCount = dimensions.x * dimensions.y;
        mem->m_width  = dimensions.x;
        mem->m_height = dimensions.y;
        return;
    }

    void set();
    
    void destroyCpuSide() {
        util::aligned_free(mem);
        return;
    }


    ParticleData& operator[](u32 idx) {
        // ifcrashstr_debug(idx >= mem->particleCount, "ParticleBufferManager::operator[](...) => Out-of-bounds Memory Access");
        return mem->buffer[idx];
    }

    size_t size()        const { return mem->m_particleCount; }
    size_t bytes()       const { return size() * sizeof(ParticleData); }
    size_t bytes_alloc() const { 
        return sizeof(ParticleBuffer) + 
        sizeof(ParticleData) * ( mem->m_particleCount - 1); 
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


struct OpenGLDataV2 {
    u32 m_fluidtex[4];
    u32 m_fboid;
    u32 m_ssboparticle;
    bool m_refreshComputeSim{false};
    bool m_refreshComputeVisual{false};
    u8   reserved[6];
    ParticleBufferManager m_inputDye;
    std::vector<vec4f>    m_inputForces;
    std::vector<vec4f>    m_initialFields;
    ShaderProgramV2 m_simulation;
    ShaderProgramV2 m_screenOutput;
};


struct GlobalStateV2
{
    AWC2::ContextID m_awc2id;
    bool            m_shouldRestartSimulation;
    bool            m_shouldRestartUserInputTexture;
    __unused u8     reserved[5];
    vec2i           m_simDims;
    frameTimeData   m_timing;
    OpenGLDataV2    m_graphics;
};


// struct GlobalState
// {
// public:
//     void create();
//     void destroy();
//     void render();


//     auto  getAWC2ID() const { return m_awc2id; }
//     auto& getMeasurementData() { return m_timing; }
//     auto& getOpenglData() { return m_graphics; }
// private:
//     void renderImGui();

//     AWC2::ContextID m_awc2id;
//     bool            m_shouldRestartSimulation;
//     bool            m_shouldRestartUserInputTexture;
//     __unused u8     reserved[5];
//     vec2i           m_simDims;
//     frameTimeData   m_timing;
//     OpenGLData      m_graphics;
// };


} // namespace GenericNamespaceName