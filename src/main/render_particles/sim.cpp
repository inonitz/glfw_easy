#include "sim.hpp"


u8 cohen_sutherland_bitcode(math::vec2f const& vec, math::vec2u const& dims)
{
    /*
        Bitcodes:
        0000 = inside
        0001 = left
        0010 = right
        0100 = bottom
        1000 = up
    */
    static const math::vec2f bounds_max{dims.x, dims.y}, bounds_min{0.0f, 0.0f};
    u8 result{0};
    result = (vec.x < bounds_min[0]);
    result |= (vec.y < bounds_min[1]) << 2;
    result |= (vec.x > bounds_max[0]) << 1;
    result |= (vec.y > bounds_max[1]) << 3;
    return result;
}




void check_particle_border_intersections(
    ParticleBuffer& particles,
    f32 dt, 
    const f32 sideLen, 
    math::vec2u const& dims
) {
    /* Cohen Sutherland algorithm for my specific edge-case */
    std::vector<ParticleData*> outOfBounds;
    math::vec2f pos, prevPos, ds, invDs;
    bool outx, outy, inside;
    u8 bcodex, last_bcodex;
    f32 min_or_max;
    static const math::vec2f bounds_max{ (dims[0] * sideLen), (dims[1] * sideLen) }, bounds[2] = {
        math::vec2f{ 0.0f, (dims[0] * sideLen) },
        math::vec2f{ 0.0f, (dims[1] * sideLen) }
    };
    static const math::vec2f bound_normal[4] = {
        math::vec2f{ 1.0f,  0.0f}, /* left   */
        math::vec2f{-1.0f,  0.0f}, /* right  */
        math::vec2f{ 0.0f,  1.0f}, /* bottom */
        math::vec2f{ 0.0f, -1.0f}  /* up     */
    };
    math::vec2f last_normal;


    for(auto& p : particles) 
    {
        pos = p.pos;
        outx = ( pos.x < bounds[0].x || pos.x > bounds[0].y );
        outy = ( pos.y < bounds[1].x || pos.y > bounds[1].y );
        if(outx || outy) 
            outOfBounds.push_back(&p);
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
        pos = p->pos;
        prevPos = pos - p->vel * dt;

        inside = bcodex = cohen_sutherland_bitcode(pos, dims);
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
            inside = cohen_sutherland_bitcode(pos, dims);
        }

        last_normal = bound_normal[__builtin_ctz(last_bcodex) - 1]; /* turn bit code into index */
        p->vel = p->vel - 2 * math::dot(p->vel, last_normal) * last_normal; /* reflect vector along normal */
        p->pos = pos;
    }


    return;
}



#ifdef __rdirprintf
void push_particles_apart(
    ParticleBuffer& particles, 
    dense_grid& pgrid, 
    f32 particleRadius
) {
    math::vec2f& p0 = particles[0].pos;
    __unused auto check_intersection = [k_radius=particleRadius](math::vec2f& a, math::vec2f& b) -> bool {
        static math::vec2f n = b - a; /* common axis */
        static f32 dist = n.length();
        static f32 dx = 2.0f * k_radius - dist;
        if (dx < 0.0f) { /* are particles overlapping */
            dist = 1.0f / dist;
            dist *= dx;
            n *= dist;
            a += n;
            b -= n;
        }
        return dx < 0.0f;
    };
    // mark(); auto begin = pgrid.as_blocks().begin();
    // mark(); auto end = pgrid.as_blocks().end();
    // // for(auto& grid : m_pgrid.as_blocks()) {
    // mark(); for(; begin != end;) {
    //     for(u16 pidx_i : begin.particle_block()) {

    //         p0 = particles[pidx_i].pos;
    //         for(u16 pidx_j : begin.particle_block())
    //         {
    //             if(pidx_j == pidx_i) {
    //                 continue;
    //             }
    //             check_intersection(p0, particles[pidx_i].pos);
    //         }
    //     }

    //     ++begin;
    // }
    __rdirprintf("pgrid.as_blocks()::begin() ==> ");
    __unused u32 i = 0;
    for(auto& grid : pgrid.as_blocks()) 
    {
        __rdirprintf("\n[block] ");
        for(u16 pidx_i : grid.particle_block()) 
        {
            __rdirprintf("%3u ", pidx_i);
            p0 = particles[pidx_i].pos;
            for(u16 pidx_j : grid.particle_block()) {
                if(pidx_j == pidx_i) {
                    continue;
                }
                // if(check_intersection(p0, particles[pidx_j].pos)) {
                //     auto* a = p0.to_string(), *b = particles[pidx_j].pos.to_string();
                //     markfmt("[block %3u] intersection %u %u\n  %s\n  %s\n", i, pidx_i, pidx_j, a, b);
                //     std::free(a);
                //     std::free(b);
                // }
            }
        }
        ++i;
    }
    __rdirprintf("\n");
    // mark();
    // for(auto& grid : pgrid.as_blocks()) 
    // {
    //     for(u16 pidx_i : grid.particle_block()) {

    //         p0 = particles[pidx_i].pos;
    //         for(u16 pidx_j : grid.particle_block()) {
    //             if(pidx_j == pidx_i) {
    //                 continue;
    //             }
    //             check_intersection(p0, particles[pidx_i].pos);
    //         }
    //     }
    // }
    // mark();
    return;
}
#endif



void simulation_step(
    ParticleBuffer& particles, 
    dense_grid& grid,
    f32 dt, 
    u32 coll_iter, 
    notused f32 rect_sidelen, 
    notused f32 particle_radius,
    notused math::vec2u const& sim_dim
) {
    static const std::array<math::vec2f, 1> actingForces = {
        math::vec2f{ 0.0f, -9.81f }
    };
    math::vec2f vel, totalForce{0.0f};
    __unused f32 sub_dt{dt / coll_iter};


    for(auto& force : actingForces) { totalForce += force; }
    for(auto& p : particles) {
        (void(&p));
        // p.vel += totalForce * dt;
        // p.pos += p.vel * dt;
    }

    //__rdirprintf("\nsimulation_step()::begin()\n");
    dt /= __scast(f32, coll_iter);
    while(coll_iter > 0) {
        // grid.print(true);
        // push_particles_apart(particles, grid, particle_radius);
        check_particle_border_intersections(particles, dt, rect_sidelen, sim_dim);
        grid.update();
        --coll_iter;
    }
    //__rdirprintf("\nsimulation_step()::end()\n");
}