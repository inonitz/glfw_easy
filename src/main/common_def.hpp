#pragma once
#include "util/vec.hpp"
#include <vector>


struct Particle
{
    math::vec2f pos;
    math::vec2f vel;
};


using ParticleBuffer = std::vector<Particle>;


