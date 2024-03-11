#pragma once
#include "util/vec.hpp"
#include "dense_grid.hpp"
#include <corecrt.h>




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
                b[i].assign(b[i].size(), value);
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
            m_weights.y[i] = 0.0f;
            m_weights.y[i + k_dimx * k_dimy] = 0.0f;
        }
        for(i32 i = 0; i < k_dimx; ++i) { /* Left & Right Boundaries */
            m_weights.x[(k_dimy + 1) * i] = 0.0f;
            m_weights.x[(k_dimy + 1) * i + k_dimy] = 0.0f;
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
        u16 begin, end;
        math::vec2f p0, pn, n;
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


    void check_particle_border_collisions(f32 dt)
    {
        // static std::vector<i32> out_bounds(m_particles.size());
        // math::vec2f bounds, tmp{1.0f}, btofl;
        // const math::vec2f neg2{-2.0f}, one{1.0f};
        // bool outx, outy;

        
        // bounds = math::vec2f{ k_dimx, k_dimy };
        // for(auto& p : m_particles)
        // {
        //     outx = ( p.pos.x > bounds.x || p.pos.x < 0.0f );
        //     outy = ( p.pos.y > bounds.y || p.pos.y < 0.0f );
        //     btofl = math::vec2f{outx, outy};
        //     tmp = one + neg2 * btofl;
        //     p.vel *= tmp;
        //     p.pos
        // }

        // const math::vec2f dir_vec[4] = {
        //     { 1.0f, 0.0f}, 
        //     {-1.0f, 0.0f}, 
        //     { 1.0f, 0.0f}, 
        //     {-1.0f, 0.0f}
        // };
        // bool dir[5]; // dud, left right down up
        // math::vec2f prevpos, delta, reflected_vel, normal;
        // f32 slope;
        // for(auto& p : m_particles) {
        //     prevpos = p.pos - p.vel * dt;
        //     delta = p.pos - prevpos;
        //     slope = delta.y / delta.x;


        //     if(delta.y == 0.0f || slope < 1.0f) {
        //         dir[2 + (p.pos.y < 0.0f) + (p.pos.y > bounds.y) * 2] = 1.0f; /* intersect X */
        //     }
        //     if(delta.x == 0.0f || slope > 1.0f) {
        //         dir[(p.pos.x < 0.0f) + (p.pos.x > bounds.x) * 2] = 1.0f; /* intersect Y */
        //     }

        //     normal = math::vec2f{0.0f};
        //     for(size_t i = 1; i < 5; ++i) {
        //         normal += dir[i] * dir_vec[i - 1];
        //     }
        //     reflected_vel = p.vel - 2 * math::dot(p.vel, normal) * normal;
        // }

        /*
        Old Note:
            for all particles p:
                Find out which wall did particle p exit through 
                    (using a line from x'n to x'n-1, where x'n = p.pos, x'n-1 = p.pos - p.vel * dt)
                get the normal of the wall n^
                update the velocity to the reflected vector along n^
                update the position to the intersection between the wall, and the line of x'n->x'n-1

            * Notes on position calculation in notebook
            * Notes on velocity calculation in code above that isn't finished (the idea is mainly there)
                (Look in notebook pages if you don't get it)
        
        New Note:
            The Schtick is basically ->
                find out which wall did the particle exit through
                    dx, dy = p.pos - (k_dimx, k_dimy)
                    if dy > dx => which vertical wall        ( + = up,    - = down)
                    else if dx > dy => which horizontal wall ( + = right, - = left)
                use the wall normal to reflect the velocity
                use the vector that is calculated using the previous iteration' position,
                    to find out where should the particle be on the border (line-line intersection + lerp)
        */
    }


    void advance_particles(f32 dt) {
        static const std::array<math::vec2f, 1> actingForces = {
            math::vec2f{ 0.0f, -9.81f }
        };
        math::vec2f vel, totalForce{0.0f};
        static math::vec2i lambda_index;
        f32 sub_dt{dt / k_collisionIterations};


        for(auto& force : actingForces) { totalForce += force; }
        for(auto& p : m_particles) {
            p.vel += totalForce * dt;
            p.pos += p.vel * dt;            
        }

        /* Sort particles since it helps in countOccurances() of m_sortedParticles() */
        std::sort(
            m_particles.begin(),
            m_particles.end(), 
            [m_gridWidth=k_dimx, &index=lambda_index](Particle const& p) {
                index = math::vec2i{p.pos}; 
                return index.j + index.i * m_gridWidth;
            }
        );


        for(size_t iter = 0; iter < k_collisionIterations; ++iter) {
            push_particles_apart(sub_dt);
            check_particle_border_collisions(sub_dt); /* Need To check collisions with border before updating data for next iteration */
            m_sortedParticles.updateInitialDataBuffer(m_swapParticles);
            m_swapParticles.swap(m_particles);
            m_sortedParticles.update();
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