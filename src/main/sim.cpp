#include "sim.hpp"
#include <cstdio>


#define __nullf32 (__scast(f32, DEFAULT32)) /* Defined as an undefined value of any vector field, i.e NULL */
#define boolexprf32(expr) __scast(f32, (expr))
#define not_null_f32_bool(float_val) boolexprf32(float_val != __nullf32)




void SimulationData::init(
    f32 unitRectangleLength, 
    u32 simulationSpaceWidth,
    u32 simulationSpaceHeight,
    u32 particleCount
) {
    k_sideLen    = unitRectangleLength;
    k_invSideLen = 1.0f / k_sideLen;
    k_particleRadius = 1.5f;
    k_ofactor   = 1.9f;
    k_stiffness = 1.0f;
    m_avgDensity = 1.0f;
    k_dimx = simulationSpaceWidth;
    k_dimy = simulationSpaceHeight;
    k_collisionIterations = 4;


    // m_particles     = std::make_unique<SimulationData::ParticleBuffer>(particleCount);
    // m_sortedParticles = std::make_unique<SimulationData::ParticleBuffer>(particleCount);
    m_particles.resize(particleCount);
    m_sortedParticles.resize(particleCount);
    for(auto& particle : m_particles) {
        particle = {
            math::vec2f{ random32f(), random32f() },
            math::vec2f{ random32f(), random32f() }
        };
        particle.pos *= unitRectangleLength * math::vec2f{ k_dimx, k_dimx };
        particle.vel *= __scast(f32, k_collisionIterations) / k_particleRadius;
    }


    markstr("   dense_grid   "); m_pgrid.create(m_particles, k_dimx, k_dimy, k_sideLen);
    markstr(" Staggered_Grid "); m_vel.create(k_dimx, k_dimy); /* Program likes to crash here too */
    markstr(" Staggered_Grid "); m_weights.create(k_dimx, k_dimy);


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
    m_density.resize(k_dimx * k_dimy);
    return;
}


void SimulationData::destroy()
{
    m_particles.resize(0);
    m_sortedParticles.resize(0);
    m_pgrid.destroy();
    m_divergence.resize(0);
    m_density.resize(0);
    m_walls.resize(0);
    m_vel.destroy();
    m_weights.destroy();
    return;
}


void SimulationData::run()
{
    constexpr u32 subSteps  = 16;
    constexpr f32 dt        = 1.0f / 60.0f;
    // constexpr f32 mixFactor = 0.9f;
    StaggeredGrid vel_copy, grid_change;


    vel_copy.create(k_dimx, k_dimy);
    grid_change.create(k_dimx, k_dimy);
    // for(u32 step = 0; step < subSteps; ++step)
    // {
    //     mark(); advance_particles(dt / subSteps);
    //     mark(); transfer_particles_to_grid();
    //     mark(); vel_copy.copy(m_vel);
    //     mark(); apply_divergence();
    //     mark(); grid_change.copy(m_vel);
    //     /* alpha * flip_method + (1 - alpha) * pic_method */
    //     mark(); vel_copy.mul(mixFactor);
    //     mark(); grid_change.sub(vel_copy);
    //     mark(); m_vel.copy(grid_change);
    //     mark(); transfer_grid_to_particles();
    // }
    mark(); advance_particles(dt / subSteps);
    mark(); transfer_particles_to_grid();
    mark(); vel_copy.copy(m_vel);


    mark(); apply_divergence();


    // for(u32 step = 0; step < subSteps; ++step)
    // {
    //     advance_particles(dt / subSteps);
    //     capture_line(transfer_particles_to_grid()     )
    //     capture_line(vel_copy.copy(m_vel)             )
    //     capture_line(apply_divergence()               )
    //     capture_line(grid_change.copy(m_vel)          )
    //     /* alpha * flip_method + (1 - alpha) * pic_method */
    //     capture_line(vel_copy.mul(mixFactor)          )
    //     capture_line(grid_change.sub(vel_copy)        )
    //     capture_line(m_vel.copy(grid_change)          )
    //     capture_line(transfer_grid_to_particles()     )
    // }
    
    
    // Time::nanosecond ntotal;
    // Time::millisecond mtotal;
    // std::vector<Time::nanosecond > per_coll_iter_ns(k_collisionIterations);
    // std::vector<Time::millisecond> per_coll_iter_ms(k_collisionIterations);
    // Time::nanosecond aggrns;
    // Time::millisecond aggrms;
    // static constexpr std::array<const char*, 14> format_str = {
    //     "  advance_particles(dt / subSteps) <=> Took %10llu[ns], %3llu[ms] / %10llu[ns], %3llu[ms] (%2.2f%%)\n",
    //     "      check_particle_border_intersections(sub_dt);       <=> Took %10llu[ns], %3llu[ms] (%2.2f%%)\n",
    //     "      push_particles_apart();                            <=> Took %10llu[ns], %3llu[ms] (%2.2f%%)\n",
    //     "      m_pGrid.updateInitialDataBuffer(*m_swapParticles); <=> Took %10llu[ns], %3llu[ms] (%2.2f%%)\n",
    //     "      m_swapParticles.swap(m_particles);                 <=> Took %10llu[ns], %3llu[ms] (%2.2f%%)\n",
    //     "      m_pGrid.update();                                  <=> Took %10llu[ns], %3llu[ms] (%2.2f%%)\n",
    //     "  transfer_particles_to_grid()     <=> Took %10llu[ns], %3llu[ms] / %10llu[ns], %3llu[ms] (%2.2f%%)\n",
    //     "  vel_copy.copy(m_vel)             <=> Took %10llu[ns], %3llu[ms] / %10llu[ns], %3llu[ms] (%2.2f%%)\n",
    //     "  apply_divergence()               <=> Took %10llu[ns], %3llu[ms] / %10llu[ns], %3llu[ms] (%2.2f%%)\n",
    //     "  grid_change.copy(m_vel)          <=> Took %10llu[ns], %3llu[ms] / %10llu[ns], %3llu[ms] (%2.2f%%)\n",
    //     "  vel_copy.mul(mixFactor)          <=> Took %10llu[ns], %3llu[ms] / %10llu[ns], %3llu[ms] (%2.2f%%)\n",
    //     "  grid_change.sub(vel_copy)        <=> Took %10llu[ns], %3llu[ms] / %10llu[ns], %3llu[ms] (%2.2f%%)\n",
    //     "  m_vel.copy(grid_change)          <=> Took %10llu[ns], %3llu[ms] / %10llu[ns], %3llu[ms] (%2.2f%%)\n",
    //     "  transfer_grid_to_particles()     <=> Took %10llu[ns], %3llu[ms] / %10llu[ns], %3llu[ms] (%2.2f%%)\n"
    // };


    // for(u32 step = 0; step < subSteps; ++step) 
    // {
    //     auto& single_capture = capture_buffer(step);
    //     ntotal = Time::nanosecond{};
    //     mtotal = Time::millisecond{};
        
        
    //     /* Take care of timestamps for total captures */
    //     printf("\nStep %u:\n", step);
    //     for(size_t i = 0; i < 4; ++i) {
    //         aggrns = Time::nanosecond{};
    //         aggrms = Time::millisecond{};
    //         for(size_t j = 0; j < 5; ++j) {
    //             aggrns += single_capture[4 * i + j];
    //             aggrms += Time::to_milli(single_capture[4 * i + j]);
    //         }
    //         per_coll_iter_ns[i] = aggrns; /* total per iteration in k_collisionIterations */
    //         per_coll_iter_ms[i] = aggrms;
    //         ntotal += aggrns; /* for advance_particles(...) */
    //         mtotal += aggrms;
    //     }
    //     aggrns = ntotal;
    //     aggrms = mtotal;
    //     for(size_t i = 20; i < 28; ++i) {
    //         ntotal += single_capture[i];
    //         mtotal += Time::to_milli(single_capture[i]);
    //     } /* finalized whole frame capture */


    //     /* print all data points */
    //     printf(format_str[0], aggrns.count(), aggrms.count(), ntotal.count(), mtotal.count(), 100.0f * __scast(f32, aggrns.count()) / ntotal.count()); /* print advance_particles(...) */
    //     for(u32 coll_iter = 0; coll_iter < k_collisionIterations; ++coll_iter) { /* Print per-collision %s */
    //         printf("    collision_iter %u <=> Took %10llu[ns], %3llu[ms] (%2.2f%%)\n", 
    //             coll_iter, 
    //             per_coll_iter_ns[coll_iter].count(), 
    //             per_coll_iter_ms[coll_iter].count(), 
    //             100.0f * __scast(f32, per_coll_iter_ns[coll_iter].count() ) / aggrns.count()
    //         );
    //         for(size_t line = 0; line < 5; ++line) {
    //             printf(format_str[line + 1], 
    //                 single_capture[4 * coll_iter + line].count(), 
    //                 Time::to_milli(single_capture[4 * coll_iter + line]).count(), 
    //                 100.0f * __scast(f32, single_capture[4 * coll_iter + line].count() ) / per_coll_iter_ns[coll_iter].count() 
    //             );
    //         }
    //     }
    //     for(size_t rem_lines = 6; rem_lines < 14; ++rem_lines) { /* Print remaining % time pts */
    //         printf(format_str[rem_lines], 
    //             single_capture[14 + rem_lines].count(), 
    //             Time::to_milli(single_capture[14 + rem_lines]).count(), 
    //             ntotal.count(),
    //             mtotal.count(),
    //             100.0f * __scast(f32, single_capture[14 + rem_lines].count() ) / ntotal.count() 
    //         );
    //     }
    // }
    return;
}





f32& SimulationData::sampleField(
    std::vector<f32>&  field,
    math::vec2i const& indices,
    bool yfield
) {
#ifdef _DEBUG
    if( !(indices.x < k_dimx && indices.y < k_dimy) ) {
        printf("!");
        // debug_messagefmt("Out of Bounds Grid Access at (%d, %d)\n", indices.x, indices.y);
        m_tmpFloat = __nullf32;
        return m_tmpFloat;
    }
    i32 idx = indices.j + indices.i * (k_dimx + yfield);
    return field[idx];
#else
    i32 idx;
    bool out_bounds;

    m_tmpFloat = __nullf32;
    out_bounds = !(indices.x < k_dimx && indices.y < k_dimy);
    idx        = indices.j + indices.i * (k_dimx + yfield);
    return out_bounds ? m_tmpFloat : field[idx]; 
#endif
}


void SimulationData::push_particles_apart()
{
    math::vec2f n;
    math::vec2f& p0 = n, &pn = n;
    f32 dist, dx;


    auto check_intersection = [&n, &dist, &dx, k_radius=k_particleRadius](math::vec2f& a, math::vec2f& b) -> void {
        n = b - a; /* common axis */
        dist = n.length();
        dx = 2.0f * k_radius - dist;
        if (dx < 0.0f) { /* are particles overlapping */
            dist = 1.0f / dist;
            dist *= dx;
            n *= dist;
            a += n;
            b -= n;
        }
    };

    
    mark(); auto begin = m_pgrid.as_blocks().begin();
    mark(); auto end = m_pgrid.as_blocks().end();
    // for(auto& grid : m_pgrid.as_blocks()) {
    mark(); for(; begin != end;) {
        mark(); for(u16 pidx_i : begin.particles()) {

            mark(); p0 = m_particles[pidx_i].pos;
            mark(); for(u16 pidx_j : begin.particles())
            {
                if(pidx_j == pidx_i) {
                    continue;
                }
                check_intersection(p0, pn = m_particles[pidx_i].pos);
            }
        }

        mark(); ++begin;
    }

    return;
}


u8 SimulationData::cohen_sutherland_bitcode(math::vec2f const& vec)
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
    result = (vec.x < bounds_min[0]);
    result |= (vec.y < bounds_min[1]) << 2;
    result |= (vec.x > bounds_max[0]) << 1;
    result |= (vec.y > bounds_max[1]) << 3;
    return result;
}


void SimulationData::check_particle_border_intersections(f32 dt)
{
    /* Cohen Sutherland algorithm for my specific edge-case */
    std::vector<Particle> outOfBounds;
    math::vec2f pos, prevPos, ds, invDs;
    bool outx, outy, inside;
    u8 bcodex, last_bcodex;
    f32 min_or_max;
    static const math::vec2f bounds_max{ (k_dimx * k_sideLen), (k_dimy * k_sideLen) }, bounds[2] = {
        math::vec2f{ 0.0f, (k_dimx * k_sideLen) },
        math::vec2f{ 0.0f, (k_dimy * k_sideLen) }
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
        outx = ( pos.x < bounds[0].x || pos.x > bounds[0].y );
        outy = ( pos.y < bounds[1].x || pos.y > bounds[1].y );
        if(outx || outy) 
            outOfBounds.push_back(p);
    }
    // debug_messagefmt("Total Particles Checked: %llu | %llu/%llu ( %f%%) Were Out Of Bounds\n",
    //     m_particles->size(), 
    //     outOfBounds.size(), 
    //     m_particles->size(), 
    //     100.0f * __scast(f32, outOfBounds.size()) / m_particles->size()   
    // );


    /* 
        Need to Check Collisions with m_walls Buffer,
        That way we can establish objects besides walls
    */
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

        last_normal = bound_normal[__builtin_ctz(last_bcodex) - 1]; /* turn bit code into index */
        p.vel = p.vel - 2 * math::dot(p.vel, last_normal) * last_normal; /* reflect vector along normal */
        p.pos = pos;
    }


    return;
}


void SimulationData::advance_particles(f32 dt) 
{
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



    for(u32 iter = 0; iter < k_collisionIterations; ++iter) {
        mark(); push_particles_apart(); 
        mark(); check_particle_border_intersections(sub_dt);
        mark(); m_pgrid.uploadSortedParticleData(m_sortedParticles);
        mark(); m_sortedParticles.swap(m_particles); /* m_particles now uses the sorted data */
        mark(); m_pgrid.update();
        // capture_line(push_particles_apart();                            ) /* Updates m_particles */
        // capture_line(check_particle_border_intersections(sub_dt);       ) /* Updates m_particles */
        // capture_line(m_pGrid.updateInitialDataBuffer(*m_swapParticles); ) /* m_swapParticles = copy(m_particles) */
        // capture_line(m_swapParticles.swap(m_particles);                 ) /* <<< refactor, it swaps the unique_ptrs (make sure this whole loop works again.) */
        // capture_line(m_pGrid.update();                                  ) /* Recompute dense hash grid */
        /* 
            tmp = m_swapParticles.data;                  // new_data
            m_swapParticles.data = m_particles.data;     // swap_container = old_data
            m_particles.data     = m_swapParticles.data; // old_data_container = new_data
        */
    }


    return;
}


void SimulationData::transfer_particles_to_grid()
{
    m_vel.set(0.0f);
    m_weights.set(0.0f);
    m_density.assign(m_density.size(), 0.0f);


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
        sampleField(m_density, index                     , false) += weights[0];
        sampleField(m_density, index + k_right           , false) += weights[1];
        sampleField(m_density, index + k_down            , false) += weights[2];
        sampleField(m_density, index + (k_down + k_right), false) += weights[3];
    }
    m_vel.div(m_weights);
    
    
    return;
}


void SimulationData::transfer_grid_to_particles()
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


    for(auto& p : m_pgrid.as_particles())
    {
        finalPos = math::vec2f{k_invSideLen} * p.pos;
        index = {
            __scast(i32, finalPos.x),
            __scast(i32, finalPos.y)
        };
        weightTotal = 0.0f;
        finalVel    = { 0.0f, 0.0f };
        for(u32 coord = 0; coord < 2; ++coord) 
        {
            const math::vec2i k_upOrDown = k_down + (2.0f * coord) * k_up;
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
                not_null_f32_bool(sampleVel[0]),
                not_null_f32_bool(sampleVel[1]),
                not_null_f32_bool(sampleVel[2]),
                not_null_f32_bool(sampleVel[3])
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


    return;
}


void SimulationData::apply_divergence()
{
    math::vec2i index;
    math::vec2f grad;
    math::vec4f wallValues;


    mark(); auto begin = m_pgrid.as_particles().begin();
    mark(); auto end = m_pgrid.as_particles().end();
    // for(auto& p : m_pgrid.as_particles())
    mark(); for(; begin != end;)
    {
        /* Can't Perform this on ALL cells - some are obstacles, some are air, etc... */
        // debug_messagefmt("particle buffer = %lld\n", 
        //     __scast(i64, (&m_particles.begin()->pos - &begin->pos) )
        // );
        grad = math::vec2f{k_invSideLen} * begin->pos;
        index = math::vec2i{grad};

        grad.x = sampleField(m_vel.b[0], { index.i    , index.j + 1 }, false) - sampleField(m_vel.b[0], index, false);
        grad.y = sampleField(m_vel.b[1], { index.i - 1, index.j     }, true)  - sampleField(m_vel.b[1], index, true); 
        grad.y += grad.x;
        grad.y *= k_ofactor;
        grad.y -= k_stiffness * ( sampleField(m_density, index, false) - m_avgDensity);
        m_divergence[index.j + index.i * k_dimx] = grad.y;
        
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
            wallValues[s] /= grad.x;
        }

        sampleField(m_vel.b[0], index                 , false) += grad.y * wallValues[0];
        sampleField(m_vel.b[0], { index.x, index.y+1 }, false) += grad.y * wallValues[1];
        sampleField(m_vel.b[1], index                 , true) += grad.y * wallValues[2];
        sampleField(m_vel.b[1], { index.x-1, index.y }, true) += grad.y * wallValues[3];
        ++begin;
    }


    return;
}
