#ifndef __COMMON_DEFINITIONS_BASE_HEADER__
#define __COMMON_DEFINITIONS_BASE_HEADER__
#include "util/vec.hpp"
#include <vector>


typedef struct __particle_data_old 
{
    math::vec2f vel{0.0f};
    math::vec2f pos{0.0f}; 
} ParticleData;


using ParticleBuffer = std::vector<ParticleData>;
using ColorBuffer    = std::vector<math::vec3f>;
#endif