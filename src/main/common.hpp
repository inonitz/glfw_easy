#ifndef __COMMON_DEFINITIONS_BASE_HEADER__
#define __COMMON_DEFINITIONS_BASE_HEADER__
#include "util/vec.hpp"


struct ParticleData 
{
    math::vec2f pos;
    math::vec2f vel;

    ParticleData() : pos{0.0f}, vel{0.0f} {}
    ParticleData(ParticleData const& cpy) {
        std::memcpy(pos.begin(), cpy.pos.begin(), sizeof(ParticleData));
    }
};


using ParticleBuffer = std::vector<ParticleData>;
using ColorBuffer    = std::vector<math::vec3f>;
#endif