#pragma once
#include "util/vec.hpp"
#include "util/random.hpp"
#include "dense_grid.hpp"
#include <vector>




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
    struct StaggeredGrid
    {
        /*
            X component Shifted down half unit
            Y component shifted right half unit
        */
        std::vector<f32> b[2];


        void create(u32 width, u32 height)
        {
            b[0].resize( (width + 1) * height );
            b[1].resize( width * (height + 1) );
            return;          
        }

        void destroy()
        {
            b[0].resize(0);
            b[1].resize(0);
            return;           
        }


        void copy(StaggeredGrid const& grid) /* structure must not be created YET */
        {
            if(grid.b[0].size() == b[0].size() && grid.b[1].size() == b[1].size()) {
                std::memcpy(b[0].data(), grid.b[0].data(), sizeof(b[0].size()));
                std::memcpy(b[1].data(), grid.b[1].data(), sizeof(b[1].size()));
            }
            return;
        }


        void set(f32 value)
        {
            for(size_t i = 0; i < 2; ++i) {
                b[i].assign(b[i].size(), value);
            }
            return;
        }


#define STAGGERED_GRID_OPERATOR(name, __src0_src1__operation_expression, __f32_expression, __f32_scalar_op) \
    void name(StaggeredGrid const& grid) { /* Assuming sizes are same */ \
        static math::vec4f __src1, __src0; \
        for(u32 index = 0; index < 2; ++index) \
        { \
            u32 i = 0; \
            for(; i < b[index].size() / 4; ++i) { \
                memcpy(__src0.begin(), &grid.b[index][4 * i], sizeof(math::vec4f)); \
                memcpy(__src1.begin(),      &b[index][4 * i], sizeof(math::vec4f)); \
                __src0_src1__operation_expression; \
                memcpy(&b[index][4 * i], __src1.begin(), sizeof(math::vec4f)); \
            } \
            i *=4 ; \
            for(; i < b[index].size(); ++i) { \
                __f32_expression; \
            } \
        } \
        return; \
    } \
    void name(f32 val) { \
        for(u32 index = 0; index < 2; ++index) \
        { \
            for(u32 i = 0; i < b[index].size(); ++i) { \
                __f32_scalar_op; \
            } \
        } \
        return; \
    } \


        STAGGERED_GRID_OPERATOR(sub, __src1 -= __src0, b[index][i] -= grid.b[index][i], b[index][i] -= val)
        STAGGERED_GRID_OPERATOR(add, __src1 += __src0, b[index][i] += grid.b[index][i], b[index][i] += val)
        STAGGERED_GRID_OPERATOR(mul, __src1 *= __src0, b[index][i] *= grid.b[index][i], b[index][i] *= val)
        STAGGERED_GRID_OPERATOR(div, __src1 *= ( math::vec4f{1.0f} / __src0 ), b[index][i] *= (1.0f / grid.b[index][i]), b[index][i] *= (1.0f / val))
    };


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
    using ParticleBuffer = std::vector<Particle>;
    std::unique_ptr<ParticleBuffer> m_particles, m_swapParticles;
    dense_grid            m_pGrid;
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