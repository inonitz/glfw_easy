#pragma once
#include "staggered.hpp"
#include "sim_grid.hpp"


class SimulationData
{
public:
    void init(
        f32 unitRectangleLength, 
        u32 simulationSpaceWidth,
        u32 simulationSpaceHeight,
        u32 particleCount
    );
    void destroy();
    void run();

private:


private:
    f32 k_sideLen;
    f32 k_invSideLen;
    f32 k_particleRadius;
    f32 k_ofactor;
    f32 k_stiffness;
    f32 m_avgDensity;
    f32 m_tmpFloat;
    i32 k_dimx;
    i32 k_dimy;
    u32 k_collisionIterations;
    /* 
        Particles in a sparse spatial hash-grid: collisions, pushing apart, drift calculations.
    */
    ParticleBuffer m_particles, m_sortedParticles;
    sim_grid       m_pgrid;
    std::vector<f32> m_divergence;
    std::vector<f32> m_density;
    std::vector<f32> m_walls;
    StaggeredGrid m_vel;
    StaggeredGrid m_weights;


    f32& sampleField(std::vector<f32>& field, math::vec2i const& indices, bool yfield);
    void push_particles_apart();
    u8 cohen_sutherland_bitcode(math::vec2f const& vec);
    void check_particle_border_intersections(f32 dt);
    void advance_particles(f32 dt);
    void transfer_particles_to_grid();
    void transfer_grid_to_particles();
    void apply_divergence();
};