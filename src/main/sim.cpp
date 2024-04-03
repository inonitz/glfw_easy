#include "sim.hpp"
#include "util/time.hpp"
#include <chrono>


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


    m_particles     = std::make_unique<SimulationData::ParticleBuffer>(particleCount);
    m_swapParticles = std::make_unique<SimulationData::ParticleBuffer>(particleCount);
    for(auto& particle : *m_particles) {
        particle = {
            math::vec2f{ random32f(), random32f() },
            math::vec2f{ random32f(), random32f() }
        };
        particle.pos *= unitRectangleLength * math::vec2f{ k_dimx, k_dimx };
        particle.vel *= __scast(f32, k_collisionIterations) / k_particleRadius;
    }


    markstr("   dense_grid   "); m_pGrid.create(*m_particles, k_dimx, k_dimy);
    markstr(" Staggered_Grid "); m_vel.create(k_dimx, k_dimy);
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
    m_particles.reset();
    m_swapParticles.reset();
    m_pGrid.destroy();
    m_divergence.resize(0);
    m_density.resize(0);
    m_walls.resize(0);
    m_vel.destroy();
    m_weights.destroy();
    return;
}


void SimulationData::run()
{
#define measure_send_buffer(timer_struct, buffer, to_exec) \
    timer_struct.tick(); \
    to_exec; \
    timer_struct.tock(); \
    buffer.push_back(timer_struct.duration()); \
    \


    Time::Timer clock;
    std::vector< Time::Timer<>::timep_dt > measuredClicks;
    constexpr u32 subSteps  = 16;
    constexpr f32 dt        = 1.0f / 60.0f;
    constexpr f32 mixFactor = 0.9f;
    StaggeredGrid vel_copy, grid_change;


    vel_copy.create(k_dimx, k_dimy);
    grid_change.create(k_dimx, k_dimy);
    for(u32 step = 0; step < subSteps; ++step)
    {
        measure_send_buffer(clock, measuredClicks, advance_particles(dt / subSteps) )
        measure_send_buffer(clock, measuredClicks, transfer_particles_to_grid()     )
        measure_send_buffer(clock, measuredClicks, vel_copy.copy(m_vel)             )
        measure_send_buffer(clock, measuredClicks, apply_divergence()               )
        measure_send_buffer(clock, measuredClicks, grid_change.copy(m_vel)          )
        /* alpha * flip_method + (1 - alpha) * pic_method */
        measure_send_buffer(clock, measuredClicks, vel_copy.mul(mixFactor)          )
        measure_send_buffer(clock, measuredClicks, grid_change.sub(vel_copy)        )
        measure_send_buffer(clock, measuredClicks, m_vel.copy(grid_change)          )
        measure_send_buffer(clock, measuredClicks, transfer_grid_to_particles()     )
    }
    
    
    Time::nanosecond ntotal;
    Time::millisecond mtotal;
    for(u32 step = 0; step < subSteps; ++step) {
        printf("Step %u:\n", step);
        
        ntotal = Time::nanosecond{};
        mtotal = Time::millisecond{};
        for(u8 substep = 0; substep < 9; ++substep) {
            ntotal += measuredClicks[step * 9 + substep];
            mtotal += Time::to_milli(measuredClicks[step * 9 + substep]);
        }
        printf("	advance_particles(dt / subSteps) <=> Took %llu[ns], %llu[ms] / %llu[ns], %llu[ms] ( %2.2f%%)\n", measuredClicks[step * 9 + 0].count(), Time::to_milli(measuredClicks[step * 9 + 0]).count(), ntotal.count(), mtotal.count(), ( __scast(f32, measuredClicks[step * 9 + 0].count()) / ntotal.count() ) );
        printf("	transfer_particles_to_grid()     <=> Took %llu[ns], %llu[ms] / %llu[ns], %llu[ms] ( %2.2f%%)\n", measuredClicks[step * 9 + 1].count(), Time::to_milli(measuredClicks[step * 9 + 1]).count(), ntotal.count(), mtotal.count(), ( __scast(f32, measuredClicks[step * 9 + 1].count()) / ntotal.count() ) );
        printf("	vel_copy.copy(m_vel)             <=> Took %llu[ns], %llu[ms] / %llu[ns], %llu[ms] ( %2.2f%%)\n", measuredClicks[step * 9 + 2].count(), Time::to_milli(measuredClicks[step * 9 + 2]).count(), ntotal.count(), mtotal.count(), ( __scast(f32, measuredClicks[step * 9 + 2].count()) / ntotal.count() ) );
        printf("	apply_divergence()               <=> Took %llu[ns], %llu[ms] / %llu[ns], %llu[ms] ( %2.2f%%)\n", measuredClicks[step * 9 + 3].count(), Time::to_milli(measuredClicks[step * 9 + 3]).count(), ntotal.count(), mtotal.count(), ( __scast(f32, measuredClicks[step * 9 + 3].count()) / ntotal.count() ) );
        printf("	grid_change.copy(m_vel)          <=> Took %llu[ns], %llu[ms] / %llu[ns], %llu[ms] ( %2.2f%%)\n", measuredClicks[step * 9 + 4].count(), Time::to_milli(measuredClicks[step * 9 + 4]).count(), ntotal.count(), mtotal.count(), ( __scast(f32, measuredClicks[step * 9 + 4].count()) / ntotal.count() ) );
        printf("	vel_copy.mul(mixFactor)          <=> Took %llu[ns], %llu[ms] / %llu[ns], %llu[ms] ( %2.2f%%)\n", measuredClicks[step * 9 + 5].count(), Time::to_milli(measuredClicks[step * 9 + 5]).count(), ntotal.count(), mtotal.count(), ( __scast(f32, measuredClicks[step * 9 + 5].count()) / ntotal.count() ) );
        printf("	grid_change.sub(vel_copy)        <=> Took %llu[ns], %llu[ms] / %llu[ns], %llu[ms] ( %2.2f%%)\n", measuredClicks[step * 9 + 6].count(), Time::to_milli(measuredClicks[step * 9 + 6]).count(), ntotal.count(), mtotal.count(), ( __scast(f32, measuredClicks[step * 9 + 6].count()) / ntotal.count() ) );
        printf("	m_vel.copy(grid_change)          <=> Took %llu[ns], %llu[ms] / %llu[ns], %llu[ms] ( %2.2f%%)\n", measuredClicks[step * 9 + 7].count(), Time::to_milli(measuredClicks[step * 9 + 7]).count(), ntotal.count(), mtotal.count(), ( __scast(f32, measuredClicks[step * 9 + 7].count()) / ntotal.count() ) );
        printf("	transfer_grid_to_particles()     <=> Took %llu[ns], %llu[ms] / %llu[ns], %llu[ms] ( %2.2f%%)\n", measuredClicks[step * 9 + 8].count(), Time::to_milli(measuredClicks[step * 9 + 8]).count(), ntotal.count(), mtotal.count(), ( __scast(f32, measuredClicks[step * 9 + 8].count()) / ntotal.count() ) );
    }
    return;
}





f32& SimulationData::sampleField(
    std::vector<f32>&  field,
    math::vec2i const& indices,
    bool yfield
) {
#ifdef _DEBUG
    if( !(indices.x < k_dimx && indices.y < k_dimy) ) {
        debug_messagefmt("Out of Bounds Grid Access at (%d, %d)\n", indices.x, indices.y);
        m_tmpFloat = __nullf32;
        return m_tmpFloat;
    }
    i32 idx = indices.j + indices.i * (k_dimx + yfield);
    return field[idx];
#else
    i32 idx;
    bool out_bounds;

    m_tmpFloat = __nullf32
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
    for(auto& block : m_pGrid.as_blocks()) 
    {
        auto particle_vector = block.get_particles();
        for(size_t i = 0; i < particle_vector.size(); ++i)
        {
            p0 = particle_vector[i].pos;
            for(size_t j = 0; j < particle_vector.size(); ++j) {
                if(i == j) continue;
                
                pn = particle_vector[j].pos;
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


    for(auto& p : *m_particles.get()) 
    {
        pos = p.pos;
        outx = ( pos.x > bounds[0].x || pos.x < bounds[0].y );
        outy = ( pos.y > bounds[1].x || pos.y < bounds[1].y );
        if(outx || outy) 
            outOfBounds.push_back(p);
    }
    debug_messagefmt("Total Particles Checked: %llu | %llu/%llu ( %f%%) Were Out Of Bounds\n",
        m_particles->size(), 
        outOfBounds.size(), 
        m_particles->size(), 
        __scast(f32, outOfBounds.size()) / m_particles->size()   
    );


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
    for(auto& p : *m_particles) {
        p.vel += totalForce * dt;
        p.pos += p.vel * dt;
    }


    for(u32 iter = 0; iter < k_collisionIterations; ++iter) {
        push_particles_apart();                           /* Updates m_particles */
        check_particle_border_intersections(sub_dt);      /* Updates m_particles */
        m_pGrid.updateInitialDataBuffer(*m_swapParticles); /* m_swapParticles = copy(m_particles) */
        m_swapParticles.swap(m_particles); // <<< refactor, it swaps the unique_ptrs (make sure this whole loop works again.)
        /* 
            tmp = m_swapParticles.data;                  // new_data
            m_swapParticles.data = m_particles.data;     // swap_container = old_data
            m_particles.data     = m_swapParticles.data; // old_data_container = new_data
        */
        m_pGrid.update(); /* Recompute dense hash grid */
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
    
    for(auto& p : *m_particles)
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


    for(auto& p : m_pGrid.as_particles())
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
    for(auto& p : m_pGrid.as_particles())
    {
        /* Can't Perform this on ALL cells - some are obstacles, some are air, etc... */
        grad = math::vec2f{k_invSideLen} * p.pos;
        index = {
            __scast(i32, grad.x),
            __scast(i32, grad.y)
        };

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
    }


    return;
}
