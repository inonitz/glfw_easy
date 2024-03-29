#pragma once
#include "util/vec.hpp"
#include "dense_grid.hpp"
#include <corecrt.h>
#include <vector>




struct SimulationData
{
    /*
        X component Shifted down half unit
        Y component shifted right half unit
    */
    struct StaggeredGrid
    {
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


        /* structure must not be created YET */
        void copy(StaggeredGrid const& grid)
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

#define STAGGERED_GRID_OPERATOR(name, op_symbol) \
        void name(StaggeredGrid const& grid) { /* Assuming sizes are same */ \
            for(u32 index = 0; index < 2; ++index) \ 
            { \
                for(u32 i = 0; i < b[index].size(); ++i) b[index][i] op_symbol##= grid.b[index][i]; \
            } \
            return; \
        } \


        STAGGERED_GRID_OPERATOR(sub, -)
        STAGGERED_GRID_OPERATOR(add, +)
        STAGGERED_GRID_OPERATOR(mul, *)
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
    i32 k_collisionIterations;
    /* 
        Particles in a sparse spatial hash-grid: collisions, pushing apart, drift calculations.
    */
    std::vector<Particle> m_particles;
    std::vector<Particle> m_swapParticles;
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
            m_weights.b[1][i] = 0.0f;
            m_weights.b[1][i + k_dimx * k_dimy] = 0.0f;
        }
        for(i32 i = 0; i < k_dimx; ++i) { /* Left & Right Boundaries */
            m_weights.b[0][(k_dimy + 1) * i] = 0.0f;
            m_weights.b[0][(k_dimy + 1) * i + k_dimy] = 0.0f;
        }


        m_walls.resize( (k_dimx + 2) * (k_dimy + 2) );
        m_walls.assign(m_walls.size(), 1.0f);
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


    void push_particles_apart(f32 dt)
    {
        auto&       sorted_data = m_sortedParticles.sorted_data();
        auto const& index_array = m_sortedParticles.hash_table();
        auto const& active_indices = m_sortedParticles.filtered_hash_table();
        math::vec2f n;
        math::vec2f& p0 = n;
        math::vec2f& pn = n;
        u16 begin, end;
        f32 dist, dx;


        for(size_t index = 1; index < active_indices.size(); ++index)
        {
            begin = index_array[ active_indices[index]  ];
            end   = index_array[ active_indices[index]+1];
            for(size_t i = begin; i < end; ++i) /* O(n^2) Collision in every block >:(((( */
            {
                p0 = m_particles[ sorted_data[i] ].pos;
                for(size_t j = begin; j < end; ++j) {
                    if(i == j) 
                        continue;
                    
                    pn = m_particles[ sorted_data[j] ].pos;
                    n = pn - p0; /* common axis */
                    dist = n.length();
                    dx = 2.0f * k_particleRadius - dist;
                    if (dx < 0.0f) { /* are particles overlapping */
                        dist = 1.0f / dist;
                        dist *= dx;
                        n *= dist;
                        p0 += n;
                        pn -= n;
                    }
                }
            }
            return;
        }
    }


    u8 cohen_sutherland_bitcode(math::vec2f const& vec)
    {
        /*
            Bitcodes:
            0000 = inside
            0001 = left
            0010 = right
            0100 = bottom
            1000 = up
        */
        static const math::vec2f bounds_max{k_dimx, k_dimy}, bounds_min{0.0f, 0.0f};
        u8 result{0};
        result |= (vec.x < bounds_min[0]);
        result |= (vec.y < bounds_min[1]) << 2;
        result |= (vec.x > bounds_max[0]) << 1;
        result |= (vec.y > bounds_max[1]) << 3;
        return result;
    }


    void check_particle_border_intersections(f32 dt)
    {
        /* Cohen Sutherland algorithm for my specific edge-case */
        std::vector<Particle> outOfBounds;
        math::vec2f pos, prevPos, ds, invDs;
        bool outx, outy, inside;
        u8 bcodex, last_bcodex;
        f32 min_or_max;
        static const math::vec2f bounds_max{k_dimx, k_dimy}, bounds[2] = {
            math::vec2f{ 0, k_dimx },
            math::vec2f{ 0, k_dimy }
        };
        static const math::vec2f bound_normal[4] = {
            math::vec2f{ 1.0f,  0.0f}, /* left   */
            math::vec2f{-1.0f,  0.0f}, /* right  */
            math::vec2f{ 0.0f,  1.0f}, /* bottom */
            math::vec2f{ 0.0f, -1.0f}  /* up     */
        };
        math::vec2f last_normal;


        for(auto& p : m_particles) 
        {
            pos = p.pos;
            outx = ( pos.x > bounds[0].x || pos.x < bounds[0].y );
            outy = ( pos.y > bounds[1].x || pos.y < bounds[1].y );
            if(outx || outy) 
                outOfBounds.push_back(p);
        }
        for(auto& p : outOfBounds)
        {
            pos = p.pos;
            prevPos = pos - p.vel * dt;

            inside = bcodex = cohen_sutherland_bitcode(pos);
            /* x_n will ALWAYS be outside, and x_n-1 inside */
            while(!inside) {
                ds = pos - prevPos;
                invDs = math::vec2f{1.0f} / ds;
                if(bcodex & 0b1100) { /* if top/bottom */
                    min_or_max = boolean(bcodex & 0b1000) * bounds_max[1];
                    pos.x = prevPos.x + ds.x * (min_or_max - prevPos.y) * invDs.y;
                    pos.y = min_or_max;

                } else if(bcodex & 0b0011) { /* right/left */
                   min_or_max = boolean(bcodex & 0b0010) * bounds_max[0];
                   pos.x = min_or_max;
                   pos.y = prevPos.y + ds.y * (min_or_max - prevPos.x) * invDs.x;
                }
                last_bcodex = inside;
                inside = cohen_sutherland_bitcode(pos);
            }

            last_normal = bound_normal[__builtin_ctz(last_bcodex) - 1];
            p.vel = p.vel - 2 * math::dot(p.vel, last_normal) * last_normal; /* reflect vector along normal */
            p.pos = pos;
        }


        return;
    }


    void advance_particles(f32 dt) {
        static const std::array<math::vec2f, 1> actingForces = {
            math::vec2f{ 0.0f, -9.81f }
        };
        math::vec2f vel, totalForce{0.0f};
        f32 sub_dt{dt / k_collisionIterations};


        for(auto& force : actingForces) { totalForce += force; }
        for(auto& p : m_particles) {
            p.vel += totalForce * dt;
            p.pos += p.vel * dt;            
        }

        for(size_t iter = 0; iter < k_collisionIterations; ++iter) {
            push_particles_apart(sub_dt);                /* Updates m_particles */
            check_particle_border_intersections(sub_dt); /* Updates m_particles */
            m_sortedParticles.updateInitialDataBuffer(m_swapParticles); /* m_swapParticles = copy(m_particles) */
            m_swapParticles.swap(m_particles); 
            /* 
                tmp = m_swapParticles.data;                  // new_data
                m_swapParticles.data = m_particles.data;     // swap_container = old_data
                m_particles.data     = m_swapParticles.data; // old_data_container = new_data
            */
            m_sortedParticles.update(); /* Recompute dense hash grid */
        }
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
                const math::vec2i k_upOrDown = coord ? k_down : k_up; /* same as (-2.0f * coord + 1.0f) * k_down */
                sampleField(m_vel.b[coord], index                         , coord) += weights[0] * p.vel[coord];
                sampleField(m_vel.b[coord], index + k_right               , coord) += weights[1] * p.vel[coord];
                sampleField(m_vel.b[coord], index + k_upOrDown            , coord) += weights[2] * p.vel[coord];
                sampleField(m_vel.b[coord], index + (k_upOrDown + k_right), coord) += weights[3] * p.vel[coord];
                sampleField(m_weights.b[coord], index                         , coord) += weights[0];
                sampleField(m_weights.b[coord], index + k_right               , coord) += weights[1];
                sampleField(m_weights.b[coord], index + k_upOrDown            , coord) += weights[2];
                sampleField(m_weights.b[coord], index + (k_upOrDown + k_right), coord) += weights[3];
            }
        }

        
        for(u32 i = 0; i < m_vel.b[0].size(); ++i) m_vel.b[0][i] *= (1.0f / m_weights.b[0][i]);
        for(u32 i = 0; i < m_vel.b[1].size(); ++i) m_vel.b[1][i] *= (1.0f / m_weights.b[1][i]);
        // for(u32 index = 0; index < 2; ++index) {
        //     for(u32 i = 0; i < m_vel.b[i].size(); ++i) {
        //         m_vel.b[index][i] *= (1.0f / m_weights.b[index][i]);
        //     }
        // }
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

                grad.x = sampleField(m_vel.b[0], { index.i    , index.j + 1 }, false) - sampleField(m_vel.b[0], index, false);
                grad.y = sampleField(m_vel.b[1], { index.i - 1, index.j     }, true)  - sampleField(m_vel.b[1], index, true); 
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

                sampleField(m_vel.b[0], index                 , false) += grad.y * wallValues[0];
                sampleField(m_vel.b[0], { index.x, index.y+1 }, false) += grad.y * wallValues[1];
                sampleField(m_vel.b[1], index                 , true) += grad.y * wallValues[2];
                sampleField(m_vel.b[1], { index.x-1, index.y }, true) += grad.y * wallValues[3];
            }
        }
    }


    void run()
    {
        constexpr u32 subSteps = 50;
        constexpr f32 dt       = 1.0f / 60.0f;
        StaggeredGrid vel_copy, grid_change;

        vel_copy.create(k_dimx, k_dimy);
        grid_change.create(k_dimx, k_dimy);
        for(u32 step = 0; step < subSteps; ++step)
        {
            advance_particles(dt / subSteps);
            transfer_particles_to_grid();
            vel_copy.copy(m_vel);
            
            apply_divergence();
            
            grid_change.copy(m_vel);
            /* grid -= __grid_tmp; */
            grid_change.sub(vel_copy);
            m_vel.copy(grid_change); /* Is supposed to be interpolated, prob 0.9 * grid_change + 0.1 * m_vel */
            transfer_grid_to_particles();
        }
        return;
    }
};