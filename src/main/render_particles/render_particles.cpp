#include "render_particles.hpp"
#include "sim.hpp"
#include "gl/camera.hpp"
#include "util/marker.hpp"
#include "dense_grid.hpp"
#include "util/time.hpp"
#include "awc/opengl.hpp"
#include "gl/shader2.hpp"
#include "util/random.hpp"


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


void random_fill_particle_buffer(ParticleBuffer& buf)
{
    for(auto& p : buf) {
        p.pos = math::vec2f{ random32f(), random32f() };
        p.vel = math::vec2f{ random32f(), random32f() };
    }
    return;
}


// void print_particles(
//     std::vector<ParticleData> const& p, 
//     std::vector<math::vec3f> const& pc
// ) {
//     for(size_t i = 0; i < p.size(); ++i) {
//         printf("[%3llu] ", i);
//         p[i].pos.print();
//         printf(" | ");
//         p[i].vel.print();
//         printf("\n");
//     }
//     for(size_t i = 0; i < pc.size(); ++i) {
//         printf("[%3llu] ", i);
//         pc[i].print();
//         printf("\n");
//     }
//     return;
// }


int render_particles()
{
    Time::Timer clock;
    ShaderProgramV2 render;
    bool keepActive;
    u8  contextID;
    __unused u32 keyCounter   = 0;
    __unused u32 frameCounter = 0;
    constexpr u32 particleCount = 256;
    f32 k_unitRectangleLength = 1.0f;
    f32 k_particleRadius      = 5.0f;
    u32 k_collisionIterations = 4;
    math::vec2u simSize{14, 20};
    math::vec2u winSize;
    math::vec2f fwinSize;
    math::vec2f fsimSize;
    math::vec2f particle_vel_const;
    math::mat2f modelMatrix;
    Camera2D sceneCamera{};

    ParticleBuffer simBufferBack;
    ParticleBuffer simBufferFront;
    ParticleBuffer drawBuffer;
    std::vector<math::vec3f> particleColors;
    dense_grid particle_grid;
    u32 vao_id;
    u32 particleDataGPU[2];


    contextID = init_lib();
    render.createFrom({
        { "main/render_particles/bshader.vert", GL_VERTEX_SHADER   },
        { "main/render_particles/bshader.frag", GL_FRAGMENT_SHADER }
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
    random_fill_particle_buffer(simBufferBack);
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
    while(keepActive)
    {
        clock.tick();
        begin_frame(); /* Begin & End Frame Are Each-Called on the Active Context! */

        winSize  = Context::windowSize(contextID);
        fwinSize = math::vec2f{ winSize[0], winSize[1] };
        sceneCamera.update(1.0f / 60.0f, fwinSize);
        // math::scale(math::vec2f{1.0f} / fwinSize, modelMatrix );

        
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
        // render.uniformMatrix2fv("modelmatrix", modelMatrix.begin());
        render.uniformMatrix4fv("modelmatrix", sceneCamera.getTransform().begin());
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