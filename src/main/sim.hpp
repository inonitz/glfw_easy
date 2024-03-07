#pragma once
#include "util/vec.hpp"
#include "dense_grid.hpp"




struct SimulationData
{
    /*
        X component Shifted down half unit
        Y component shifted right half unit
    */
    struct StaggeredGrid
    {
        union {
            std::vector<f32> b[2];
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
            struct {
                std::vector<f32> x;
                std::vector<f32> y;
            };
            #pragma GCC diagnostic pop
        };


        void create(u32 width, u32 height)
        {
            x.resize( (width + 1) * height );
            y.resize( width * (height + 1) );
            return;
        }


        /* structure must not be created YET */
        void copy(StaggeredGrid const& grid)
        {
            if(grid.x.size() == x.size() && grid.y.size() == y.size()) {
                std::memcpy(x.data(), grid.x.data(), sizeof(x.size()));
                std::memcpy(y.data(), grid.y.data(), sizeof(y.size()));
            }
            return;
        }


        void set(f32 value)
        {
            for(size_t i = 0; i < 2; ++i) {
                b[i].assign(value, b[i].size());
            }
            return;
        }
    };


    f32 k_sideLen;
    f32 k_invSideLen;
    f32 k_particleRadius;
    f32 k_ofactor;
    f32 k_stiffness;
    f32 m_avgDensity;
    f32 m_tmpFloat;
    i32 k_dimx;
    i32 k_dimy;
    /* 
        Store Particles in a sparse spatial hash-grid,
        that way you know to check for collisions,
        and to push particles apart, aside from basic drift calculations.
    */
    std::vector<Particle> m_particles;
    dense_grid            m_sortedParticles;
    std::vector<f32> m_divergence;
    std::vector<f32> m_density;
    std::vector<f32> m_walls;
    StaggeredGrid m_vel;
    StaggeredGrid m_weights;


    void init(
        f32 unitRectangleLength, 
        u32 simulationSpaceWidth,
        u32 simulationSpaceHeight,
        u32 particleCount
    ) {
        k_sideLen    = unitRectangleLength;
        k_invSideLen = 1.0f / k_sideLen;
        k_particleRadius = 1.0f;
        k_ofactor   = 1.9f;
        k_stiffness = 1.0f;
        k_dimx = simulationSpaceWidth;
        k_dimy = simulationSpaceHeight;


        m_sortedParticles.create(m_particles, k_dimx, k_dimy);
        m_vel.create(k_dimx, k_dimy);
        m_weights.create(k_dimx, k_dimy);
        

        /* Set Border Variables to 0. */
        for(i32 i = 0; i < k_dimx; ++i) { /* Top & Bottom Boundaries */
            m_weights.y[i] = 0.0f;
            m_weights.y[i + k_dimx * k_dimy] = 0.0f;
        }
        for(i32 i = 0; i < k_dimx; ++i) { /* Left & Right Boundaries */
            m_weights.x[(k_dimy + 1) * i] = 0.0f;
            m_weights.x[(k_dimy + 1) * i + k_dimy] = 0.0f;
        }


        m_walls.resize( (k_dimx + 2) * (k_dimy + 2) );
        m_walls.assign(1.0f, m_walls.size());
        for(i32 j = 0; j < k_dimy + 2; ++j) {
            m_walls[j] = 0.0f;
            m_walls[j + (k_dimx + 1) * (k_dimy + 2)] = 0.0f;
        }
        for(i32 i = 1; i < k_dimx; ++i) {
            m_walls[(k_dimy + 2) * i] = 0.0f;
            m_walls[(k_dimy + 2) * i + k_dimy + 1] = 0.0f;
        }

        m_divergence.resize(k_dimx * k_dimy);


        return;
    }


    // math::vec4f gridAt(math::vec2i const& indices)
    // {
    //     /* X component | Y component */
    //     /* indices.i < dimx, */
    //     if( !(indices.x < dimx && indices.y < dimy) ) {
    //         debug_messagefmt("Out of Bounds Grid Access at (%d, %d)\n", indices.x, indices.y); 
    //         return math::vec4f{0.0f};
    //     }
    //     return math::vec4f{
    //         /* Left, Right, Down, Up */
    //         vel.x[indices.j + indices.i * (dimx + 1)], vel.x[indices.j +  1   + indices.i * (dimx + 1)],
    //         vel.y[indices.j + indices.i *     dimx  ], vel.y[indices.j + dimx + indices.i *     dimx  ]
    //     };
    // }


    f32& sampleField(
        std::vector<f32>&  field,
        math::vec2i const& indices,
        bool yfield
    ) {
#ifdef _DEBUG
        if( !(indices.x < k_dimx && indices.y < k_dimy) ) {
            debug_messagefmt("Out of Bounds Grid Access at (%d, %d)\n", indices.x, indices.y);
            m_tmpFloat = __scast(f32, DEFAULT32);
            return m_tmpFloat;
        }
        i32 idx = indices.j + indices.i * (k_dimx + yfield);
        return field[idx];
#else
        i32 idx;
        bool out_bounds;

        m_tmpFloat = __scast(f32, DEFAULT32);
        out_bounds = !(indices.x < k_dimx && indices.y < k_dimy);
        idx        = indices.j + indices.i * (k_dimx + yfield);
        return out_bounds ? m_tmpFloat : field[idx]; 
#endif
    }


    void push_particles_apart()
    {
        /* 
            1. solve boundary conditions: The world is a rectangle 
                (X axis is right, Y axis is up) 
                with the dimensions of the simulation  
            2. Solve particle collisions
                use dense_grid to find particle collisions &
                re-distribute the particles
        */
        static std::vector<i32> out_bounds(m_particles.size());

        math::vec2f bounds = math::vec2f{ k_dimx, k_dimy };
        bool outx, outy;
        for(auto& p : m_particles)
        {
            outx = ( p.pos.x > bounds.x || p.pos.x < 0.0f );
            outy = ( p.pos.y > bounds.y || p.pos.y < 0.0f );

        }
    }


    void advance_particles(f32 dt) {
        static const std::array<math::vec2f, 1> actingForces = {
            math::vec2f{ 0.0f, -9.81f }
        };
        math::vec2f vel, totalForce{0.0f};

        for(auto& force : actingForces) { totalForce += force; }
        for(auto& p : m_particles) {
            p.vel += totalForce * dt;
            p.pos += p.vel * dt;            
        } /* This will scramble the data because indices have now updated. */
        
        
        m_sortedParticles.update();

        /* TODO: Push particles out of obstacles - use the spatial-hash grid i Mentioned. */
        return;
    }


    void transfer_particles_to_grid()
    {
        m_vel.set(0.0f);
        m_weights.set(0.0f);

        math::vec4f weights;
        math::vec2f indexf;
        math::vec2f delta, omdelta;
        math::vec2i index;
        const math::vec2i 
            k_right = { 0, 1 },
            k_up    = {-1, 0 },
            k_down  = { 1, 0 };
        
        for(auto& p : m_particles)
        {
            indexf = math::vec2f{k_invSideLen} * p.pos;
            indexf = {
                std::floorf(indexf.x),
                std::floorf(indexf.y)
            };
            index = {
                __scast(i32, indexf.x),
                __scast(i32, indexf.y)
            };
            delta   = p.pos - indexf * k_sideLen;
            omdelta = math::vec2f{1.0f} - delta;
            weights = {
                omdelta.x * omdelta.y,
                delta.x   * omdelta.y,
                delta.x   * delta.y,
                omdelta.x * delta.y
            };


            for(size_t coord = 0; coord < 2; ++coord) {
                const math::vec2i k_upOrDown = coord ? k_down : k_up;
                sampleField(m_vel.b[coord], index                         , coord) += weights[0] * p.vel[coord];
                sampleField(m_vel.b[coord], index + k_right               , coord) += weights[1] * p.vel[coord];
                sampleField(m_vel.b[coord], index + k_upOrDown            , coord) += weights[2] * p.vel[coord];
                sampleField(m_vel.b[coord], index + (k_upOrDown + k_right), coord) += weights[3] * p.vel[coord];
                sampleField(m_weights.b[coord], index                         , coord) += weights[0];
                sampleField(m_weights.b[coord], index + k_right               , coord) += weights[1];
                sampleField(m_weights.b[coord], index + k_upOrDown            , coord) += weights[2];
                sampleField(m_weights.b[coord], index + (k_upOrDown + k_right), coord) += weights[3];
            }
            /*
                * The following paragraph should solve the following functions:
                    * apply_divergence();
                    * transfer_grid_to_particles();
                * The paragraph does NOT mention a solution for the following functions:
                    * handle_collisions() [Probably just use a Hash table for collision detection in cells]
                    * push_particles_apart( relevant_regions... )
                * I need to keep a set (called 'water_cell_indices') of all indices that have been accessed/modified during this particle->grid interaction,
                * s.t I'll be able to know which cells contain water and the rest are Air/Walls - This Set Needs to be cleared & updated during every advance_particles() update.
                * [NOTE]: I could also store them in a sparse spatial-hash grid, but I'm not so sure the maintenance/upkeep
                * of such a structure, is necessarily needed/worth it for this simulation. Maybe it'll come in handy in regards
                * to handling of particle collisions
                * Anyway, now that I have the 'water_indices' set, I'll be able to know where to apply divergence.
                * Moreover, because 'grid_change' and 'velCopy' access the same blocks of fluid in the grid,
                * There shouldn't be a reason to re-calculate 'water_indices', since no new blocks of fluid have appeared.
                * Finally, With regards to the Drift-Problem: use a staggered-grid/centroid (dimx+1, dimy+1) matrix for calculation of densities at cell-centers,
                * s.t the divergence can be fixed for the drift-problem; Still, particle-collisions need to be handled properly (push_particles_apart())
            */
        }


        for(u32 i = 0; i < m_vel.x.size(); ++i) {
            m_vel.x[i] *= (1.0f / m_weights.x[i]);
        }
        for(u32 i = 0; i < m_vel.y.size(); ++i) {
            m_vel.y[i] *= (1.0f / m_weights.y[i]);
        }
        return;
    }


    void transfer_grid_to_particles()
    {
        math::vec2i index;
        math::vec4f sampleWeights;
        math::vec4f sampleVel;
        
        math::vec2f finalVel;
        math::vec2f finalPos;
        math::vec4f wellDefinedSample;
        f32 weightTotal = 0.0f;


        const math::vec2i 
            k_right = { 0, 1 },
            k_up    = {-1, 0 },
            k_down  = { 1, 0 };
    
        /* Needs to iterate over all water cells - not all cells */
        for(i32 i = 0; i < k_dimx; ++i) {
            for(i32 j = 0; j < k_dimy; ++j) {
                index = { i, j };
                weightTotal = 0.0f;
                finalVel    = { 0.0f, 0.0f };
                for(u32 coord = 0; coord < 2; ++coord) 
                {
                    const math::vec2i k_upOrDown = coord ? k_down : k_up;
                    sampleVel = {
                        sampleField(m_vel.b[coord], index                         , coord),
                        sampleField(m_vel.b[coord], index + k_right               , coord),
                        sampleField(m_vel.b[coord], index + k_upOrDown            , coord),
                        sampleField(m_vel.b[coord], index + (k_upOrDown + k_right), coord)
                    };
                    sampleWeights = {
                        sampleField(m_weights.b[coord], index                         , coord),
                        sampleField(m_weights.b[coord], index + k_right               , coord),
                        sampleField(m_weights.b[coord], index + k_upOrDown            , coord),
                        sampleField(m_weights.b[coord], index + (k_upOrDown + k_right), coord)
                    };
                    wellDefinedSample = {
                        (sampleVel[0] == __scast(f32, DEFAULT32) ) ? 0.0f : 1.0f,
                        (sampleVel[1] == __scast(f32, DEFAULT32) ) ? 0.0f : 1.0f,
                        (sampleVel[2] == __scast(f32, DEFAULT32) ) ? 0.0f : 1.0f,
                        (sampleVel[3] == __scast(f32, DEFAULT32) ) ? 0.0f : 1.0f
                    };
                    
                    
                    for(u32 s = 0; s < 4; ++s) {
                        weightTotal += wellDefinedSample[s] * sampleWeights[s]; 
                    }
                    weightTotal = 1.0f / weightTotal;

                    for(u32 s = 0; s < 4; ++s) {
                        finalVel[coord] += wellDefinedSample[s] * sampleVel[s] * sampleWeights[s];
                    }
                    finalVel *= weightTotal;
                }
            }
        }


        return;
    }


    void apply_divergence()
    {
        math::vec2i index;
        math::vec2f grad;
        math::vec4f wallValues;
        for(i32 i = 1; i < k_dimx; ++i) {
            for(i32 j = 1; j < k_dimy; ++j) {
                /* Can't Perform this on ALL cells - some are obstacles, some are air, etc... */
                index = { i, j };

                grad.x = sampleField(m_vel.x, { index.i    , index.j + 1 }, false) - sampleField(m_vel.x, index, false);
                grad.y = sampleField(m_vel.y, { index.i - 1, index.j     }, true)  - sampleField(m_vel.y, index, true); 
                grad.y += grad.x;
                grad.y *= k_ofactor;
                m_divergence[j + i * k_dimx] = grad.y;
                
                wallValues = {
                     1.0f * sampleField(m_walls, { index.i, index.j - 1 }, false),
                    -1.0f * sampleField(m_walls, { index.i, index.j + 1 }, false),
                     1.0f * sampleField(m_walls, { index.i + 1, index.j }, false),
                    -1.0f * sampleField(m_walls, { index.i - 1, index.j }, false),
                };
                grad.x = 0.0f;
                for(u32 s = 0; s < 4; ++s) {
                    grad.x += wallValues[s];
                }
                for(u32 s = 0; s < 4; ++s) {
                    wallValues[i] /= grad.x;
                }

                sampleField(m_vel.x, index                  , false) += grad.y * wallValues[0];
                sampleField(m_vel.x, { index.x, index.y+1 } , false) += grad.y * wallValues[1];
                sampleField(m_vel.y, index                  , true) += grad.y * wallValues[2];
                sampleField(m_vel.y, { index.x-1, index.y } , true) += grad.y * wallValues[3];
            }
        }
    }


    void run()
    {
        constexpr u32 subSteps = 50;
        constexpr f32 dt       = 1.0f / 60.0f;
        StaggeredGrid* vel_copy, *grid_change;


        vel_copy    = amalloc_t(StaggeredGrid, sizeof(StaggeredGrid), round2(sizeof(StaggeredGrid)));
        grid_change = amalloc_t(StaggeredGrid, sizeof(StaggeredGrid), round2(sizeof(StaggeredGrid)));
        
        vel_copy->create(k_dimx, k_dimy);
        grid_change->create(k_dimx, k_dimy);
        for(u32 step = 0; step < subSteps; ++step)
        {
            advance_particles(dt / subSteps);
            transfer_particles_to_grid();
            vel_copy->copy(m_vel);
            
            apply_divergence();
            
            grid_change->copy(m_vel);
            /* grid -= __grid_tmp; */
            for(u32 i = 0; i < grid_change->x.size(); ++i) grid_change->x[i] -= vel_copy->x[i];
            for(u32 i = 0; i < grid_change->y.size(); ++i) grid_change->y[i] -= vel_copy->y[i];
            m_vel.copy(*grid_change); /* Is supposed to be interpolated, prob 0.9 * grid_change + 0.1 * m_vel */
            transfer_grid_to_particles();
        }
        return;
    }
};


/*



def coords(i, j, dimx, dimy):
    return [j + (dimx+1) * i, j+1 + (dimx+1) * i, j + dimx * i + dimx, j + dimx * i]




void run_simulation()
{
    StaggeredGrid velCopy;
    advect_particles(dt_substep) {
        totalForce = [vec2(f) for f in forces].sum();
        for(particle p in m_particles) {
            p.vel += totalForce * dt_substep;
            p.pos += p.vel * dt_substep;
        }

        for(auto& block : grid) {
            if(block.collision()) {
                for(auto& particle : m_particles) {
                    particle.pos -= 2 * particle.vel * dt_substep;
                }
            }
        }

    }
    transfer_particles_to_grid();
    velCopy = m_vel;
    apply_projection();
    
    grid_change = m_vel - velCopy;
    transfer_grid_to_particles(grid_change * alpha + (1.0f - alpha) * m_vel);
}

*/