#include "render_particles.hpp"
#include "awc/inputdef.hpp"
#include "util/base.hpp"
#include "util/time.hpp"
#include "awc/awc.hpp"
#include "awc/windowdef.hpp"
#include "awc/opengl.hpp"
#include "gl/shader2.hpp"
#include "util/random.hpp"
#include "util/vec.hpp"


struct ParticleData {
    math::vec2f pos;
    math::vec2f vel;
};



using namespace AWC;


u8 init_lib();


int render_particles()
{
    Time::Timer clock;
    ShaderProgramV2 render;
    bool keepActive;
    u8  contextID;
    __unused u32 keyCounter   = 0;
    __unused u32 frameCounter = 0;
    constexpr u32 particleCount = 16384;
    f32 k_unitRectangleLength = 1.0f;
    f32 k_particleRadius      = 5.0f;
    u32 k_collisionIterations = 4;
    std::vector<ParticleData> particles;
    std::vector<math::vec3f> particleColors;
    math::mat2f modelMatrix;

    u32 vao_id;
    u32 particleDataGPU[2];


    contextID = init_lib();
    render.createFrom({
        { "misc/shaders/default/bshader.vert", GL_VERTEX_SHADER   },
        { "misc/shaders/default/bshader.frag", GL_FRAGMENT_SHADER }
    });
    markfmt("Render Shader Status: %u\n", render.compile());


    particles.resize(particleCount);
    particleColors.resize(particleCount);
    auto winSize  = Context::windowSize(contextID);
    auto fwinSize = math::vec2f{ winSize[0], winSize[1] };

    auto l_refresh_particles = [
        &fwinSize, 
        &k_unitRectangleLength,
        &k_collisionIterations, 
        &k_particleRadius
        ](std::vector<ParticleData>& particles) -> void 
    {
        auto scale_factor = k_unitRectangleLength * fwinSize;
        auto vel_factor   = __scast(f32, k_collisionIterations) / k_particleRadius;
        for(size_t i = 0; i < particles.size(); ++i) {
            particles[i] = {
                math::vec2f{-1.0f} + 2.0f * math::vec2f{ random32f(), random32f() },
                math::vec2f{-1.0f} + 2.0f * math::vec2f{ random32f(), random32f() }
            };
            particles[i].pos *= scale_factor;
            particles[i].vel *= vel_factor;
        }
        return;
    };

    l_refresh_particles(particles);
    for(auto& pcol : particleColors) { pcol = math::vec3f{ random32f(), random32f(), random32f() }; }
    // for(size_t i = 0; i < particleCount; ++i) {
    //     printf("[%3llu] ", i);
    //     particles[i].pos.print();
    //     printf(" | ");
    //     particles[i].vel.print();
    //     printf("\n");
    // }
    // for(size_t i = 0; i < particleCount; ++i) {
    //     printf("[%3llu] ", i);
    //     particleColors[i].print();
    //     printf("\n");
    // }

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
    auto l_refresh_gpu_particle_pos = [](u32 bufid, std::vector<ParticleData>& pbuf) -> void {
        __glcheck( 
            gl()->NamedBufferData(bufid, 
            pbuf.size() * sizeof(ParticleData), 
            pbuf.data(), 
            GL_DYNAMIC_DRAW
        ));
        return;
    };
    l_refresh_gpu_particle_pos(particleDataGPU[0], particles);
    __glcheck( gl()->NamedBufferData(particleDataGPU[1], 
        particleColors.size() * sizeof(math::vec3f), 
        particleColors.data(), 
        GL_DYNAMIC_DRAW
    ); )


    
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
        math::scale(math::vec2f{1.0f} / fwinSize, modelMatrix );
        render.uniformMatrix2fv("modelmatrix", modelMatrix.begin());

        k_particleRadius += 1.0f * Input::isMouseButtonPressed(Input::mouseButton::LEFT);
        k_particleRadius -= 1.0f * Input::isMouseButtonPressed(Input::mouseButton::RIGHT);
        render.uniform1f("particleSize", k_particleRadius);

        if(Input::isKeyRepeated(Input::keyCode::R) || Input::isKeyPressed(Input::keyCode::R)) {
            l_refresh_particles(particles);
            l_refresh_gpu_particle_pos(particleDataGPU[0], particles);
        }

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
    particles.resize(0);
    particleColors.resize(0);
    destroy();
    return 0;
}




u8 init_lib()
{
    u8 ctxid;
    
    init();
    ifcrash( (ctxid = Context::allocate() ) == NULL);
    
    Context::setActive(ctxid);
    Context::init(
        WindowOptions{{{
            WINDOW_FRAMEBUFFER_BITS_DEFAULT, 
            WINDOW_OPTION_DEFAULT | WINDOW_OPTION_RESIZABLE, 
            144, 
            0 
        }}},
        WindowDescriptor{ {{ 1080u, 720u }}, nullptr }
    );
    return ctxid;
}