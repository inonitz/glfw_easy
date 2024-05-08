#include "render_particles.hpp"
#include "main/common.hpp"
#include "util/base.hpp"
#include "util/marker.hpp"
#include "main/dense_grid.hpp"
#include "util/time.hpp"
#include "awc/opengl.hpp"
#include "gl/shader2.hpp"
#include "util/random.hpp"
#include <corecrt.h>


using namespace AWC;


template<typename T> void refresh_gpu_data(u32 bufid, std::vector<T>& buf) {
    __glcheck(
        gl()->NamedBufferData(bufid, 
            buf.size() * sizeof(buf.data()[0]), 
            buf.data(), 
            GL_DYNAMIC_DRAW
    ));
    return;
};


void transform_screen_space(std::vector<ParticleData>& buf) 
{
    const auto minus_one = math::vec2f{-1.0f};
    const auto two       = math::vec2f{2.0f};
    for(size_t i = 0; i < buf.size(); ++i) {
        buf[i].pos = minus_one + two * buf[i].pos;
        buf[i].vel = minus_one + two * buf[i].vel;
    }
    return;
}


void fill_random_data(ParticleBuffer& buf)
{
    for(auto& p : buf) {
        p.pos = math::vec2f{ random32f(), random32f() };
        p.vel = math::vec2f{ random32f(), random32f() };
    }
    return;
}


void print_particles(
    std::vector<ParticleData> const& p, 
    std::vector<math::vec3f> const& pc
) {
    for(size_t i = 0; i < p.size(); ++i) {
        printf("[%3llu] ", i);
        p[i].pos.print();
        printf(" | ");
        p[i].vel.print();
        printf("\n");
    }
    for(size_t i = 0; i < pc.size(); ++i) {
        printf("[%3llu] ", i);
        pc[i].print();
        printf("\n");
    }
    return;
}


void simulation_step(
    ParticleBuffer& particles, 
    dense_grid& grid,
    f32 dt, 
    u32 coll_iter, 
    f32 rect_sidelen, 
    f32 particle_radius,
    math::vec2u const& sim_dim
);




int render_particles()
{
    Time::Timer clock;
    ShaderProgramV2 render;
    bool keepActive;
    u8  contextID;
    __unused u32 keyCounter   = 0;
    __unused u32 frameCounter = 0;
    constexpr u32 particleCount = 512;
    f32 k_unitRectangleLength = 16.0f;
    f32 k_particleRadius      = 5.0f;
    u32 k_collisionIterations = 16;
    math::vec2u simSize{32, 32};
    math::vec2u winSize;
    math::vec2f fwinSize;
    math::vec2f fsimSize;
    math::vec2f particle_vel_const;
    math::mat2f modelMatrix;
    
    ParticleBuffer simBufferBack;
    ParticleBuffer simBufferFront;
    ParticleBuffer drawBuffer;
    std::vector<math::vec3f> particleColors;
    dense_grid particle_grid;
    u32 vao_id;
    u32 particleDataGPU[2];


    contextID = init_lib();
    render.createFrom({
        { "misc/shaders/default/bshader.vert", GL_VERTEX_SHADER   },
        { "misc/shaders/default/bshader.frag", GL_FRAGMENT_SHADER }
    });
    __unused auto status = render.compile();
    markfmt("Render Shader Status: %u\n", status);


    simBufferBack.resize(particleCount);
    simBufferFront.resize(particleCount);
    drawBuffer.resize(particleCount);
    particleColors.resize(particleCount);
    winSize  = Context::windowSize(contextID);
    fwinSize = math::vec2f{ winSize[0], winSize[1] };
    fsimSize = math::vec2f{ simSize[0], simSize[1] } * k_unitRectangleLength;
    particle_vel_const = math::vec2f{ __scast(f32, k_collisionIterations) / k_particleRadius };
    /* Fill simulation Buffer before simulation_step() */
    fill_random_data(simBufferBack);
    for(auto& p : simBufferBack) {
        p.pos *= fsimSize;
        p.vel *= particle_vel_const;
    }
    /* Fill ParticleColors Once for draw Call */
    for(auto& pcol : particleColors) { 
        pcol = math::vec3f{ random32f(), random32f(), random32f() }; 
    }
    particle_grid.create(&simBufferBack, simSize.x, simSize.y, k_unitRectangleLength);


    gl()->CreateVertexArrays(1, &vao_id);
    gl()->CreateBuffers(2, particleDataGPU);
    /* Vertex Array State */
    __glcheck( gl()->BindVertexArray(vao_id);)
    __glcheck( gl()->VertexArrayVertexBuffer(vao_id, 0, particleDataGPU[0], 0, sizeof(ParticleData));)
    __glcheck( gl()->VertexArrayVertexBuffer(vao_id, 1, particleDataGPU[1], 0, sizeof(math::vec3f));)
    __glcheck( gl()->EnableVertexArrayAttrib(vao_id, 0);)
    __glcheck( gl()->EnableVertexArrayAttrib(vao_id, 1);)
    __glcheck( gl()->EnableVertexArrayAttrib(vao_id, 2);)
    __glcheck( gl()->VertexArrayAttribFormat(vao_id, 0, 2, GL_FLOAT, false, offsetof(ParticleData, pos));)
    __glcheck( gl()->VertexArrayAttribFormat(vao_id, 1, 2, GL_FLOAT, false, offsetof(ParticleData, vel));)
    __glcheck( gl()->VertexArrayAttribFormat(vao_id, 2, 3, GL_FLOAT, true,  0);)
    __glcheck( gl()->VertexArrayAttribBinding(vao_id, 0, 0);)
    __glcheck( gl()->VertexArrayAttribBinding(vao_id, 1, 0);)
    __glcheck( gl()->VertexArrayAttribBinding(vao_id, 2, 1);)
    /* Vertex Buffer State */
    refresh_gpu_data(particleDataGPU[1], particleColors);


    gl()->Enable(GL_PROGRAM_POINT_SIZE);
    gl()->Enable(GL_BLEND); 
    gl()->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    gl()->ClearColor(0.0f, 0.5f, 0.7f, 1.0f);
    keepActive = Context::windowActive(contextID);


    render.bind();
    while(keepActive && frameCounter < 16)
    {
        clock.tick();
        begin_frame(); /* Begin & End Frame Are Each-Called on the Active Context! */

        winSize  = Context::windowSize(contextID);
        fwinSize = math::vec2f{ winSize[0], winSize[1] };
        math::scale(math::vec2f{1.0f} / fwinSize, modelMatrix );

        
        simulation_step(
            simBufferBack, 
            particle_grid, 
            1.0f/60.0f, 
            k_collisionIterations, 
            k_unitRectangleLength, 
            k_particleRadius, 
            simSize
        );
        particle_grid.uploadSortedParticleData(drawBuffer);
        particle_grid.uploadSortedParticleData(simBufferFront);
        simBufferFront.swap(simBufferBack);
        transform_screen_space(drawBuffer);
        refresh_gpu_data(particleDataGPU[0], drawBuffer);
        // k_particleRadius += 1.0f * Input::isMouseButtonPressed(Input::mouseButton::LEFT);
        // k_particleRadius -= 1.0f * Input::isMouseButtonPressed(Input::mouseButton::RIGHT);
        // particle_vel_const = math::vec2f{ __scast(f32, k_collisionIterations) / k_particleRadius };
        // if(Input::isKeyRepeated(Input::keyCode::R) || Input::isKeyPressed(Input::keyCode::R)) {
        //     fwinSize *= math::vec2f{ k_unitRectangleLength };
        //     fill_random_data(particles);
        //     for(auto& p : drawBuffer) {
        //         p.pos *= k_unitRectangleLength * fwinSize;
        //         p.vel *= particle_vel_const;
        //     }
        //     transform_screen_space(particles);
        //     refresh_gpu_data(particleDataGPU[0], particles);
        // }


        /* Bind Uniforms */
        render.uniformMatrix2fv("modelmatrix", modelMatrix.begin());
        render.uniform1f("particleSize", k_particleRadius);
        /* Draw Call */
        __glcheck( gl()->DrawArrays(GL_POINTS, 0, particleCount); );


        /* Happens before end_frame() because Input is reset by then */
        keepActive = Context::windowActive(contextID);
        keepActive = keepActive && !Input::isKeyPressed(Input::keyCode::ESCAPE);
        end_frame();
        clock.tock();
        ++frameCounter;
    }


    render.destroy();
    gl()->DeleteVertexArrays(1, &vao_id);
    gl()->DeleteBuffers(2, particleDataGPU);
    drawBuffer.resize(0);
    simBufferBack.resize(0);
    particleColors.resize(0);
    destroy();
    return 0;
}


void push_particles_apart(
    ParticleBuffer& particles, 
    dense_grid& pgrid, 
    f32 particleRadius
);
u8 cohen_sutherland_bitcode(math::vec2f const& vec, math::vec2u const& dims);
void check_particle_border_intersections(
    ParticleBuffer& particles,
    f32 dt, 
    const f32 sideLen, 
    math::vec2u const& dims
);


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

    __rdirprintf("\nsimulation_step()::begin()\n");
    dt /= __scast(f32, coll_iter);
    while(coll_iter > 0) {
        grid.print();
        push_particles_apart(particles, grid, particle_radius);
        check_particle_border_intersections(particles, dt, rect_sidelen, sim_dim);
        grid.update();
        --coll_iter;
    }
    __rdirprintf("\nsimulation_step()::end()\n");
}



void push_particles_apart(
    ParticleBuffer& particles, 
    dense_grid& pgrid, 
    f32 particleRadius
) {
    math::vec2f& p0 = particles[0].pos;
    auto check_intersection = [k_radius=particleRadius](math::vec2f& a, math::vec2f& b) -> void {
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
    for(auto& grid : pgrid.as_blocks()) 
    {
        __rdirprintf("\n    block: ");
        for(u16 pidx_i : grid.particle_block()) 
        {
            __rdirprintf("%3u ", pidx_i);
            p0 = particles[pidx_i].pos;
            for(u16 pidx_j : grid.particle_block()) {
                if(pidx_j == pidx_i) {
                    continue;
                }
                check_intersection(p0, particles[pidx_i].pos);
            }
        }
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