#pragma once
#include "common.hpp"
#include "dense_grid.hpp"


u8 cohen_sutherland_bitcode(math::vec2f const& vec, math::vec2u const& dims);
void check_particle_border_intersections(
    ParticleBuffer& particles,
    f32 dt, 
    const f32 sideLen, 
    math::vec2u const& dims
);


void push_particles_apart(
    ParticleBuffer& particles, 
    dense_grid& pgrid, 
    f32 particleRadius
);


void simulation_step(
    ParticleBuffer& particles, 
    dense_grid& grid,
    f32 dt, 
    u32 coll_iter, 
    notused f32 rect_sidelen, 
    notused f32 particle_radius,
    notused math::vec2u const& sim_dim
);