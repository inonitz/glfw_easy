#include "test_func.hpp"
#include "awc/awc.hpp"
#include "awc/opengl.hpp"
#include "util/count.hpp"
#include "util/time.hpp"
#include <util/vec.hpp>
#include "gl/shader2.hpp"
#include "gl/vertexArray.hpp"
#include "gl/texture.hpp"
#include "util/image.hpp"


using namespace AWC;

/*
    Draw Particles to specific positions
    Draw Boundaries (Rectangle)
    Draw Grid (debug)

    Control Particle Position using a buffer
    Update the Positions using Compute shader (Simulation Occurs on the GPU)
    - No need to retrieve anything to the CPU, 
        everything can happen directly on the GPU

    Simulation Needs Initial Parameters to start,
    Probably passed as uniforms/In a UBO
*/


u8 initialize();


int test_functionality()
{
    Time::Timer clock;
    u8          contextID;
    bool        keepActive = false;
    u32         keyCounter = 0;
    u32         frameCounter = 0;
    // u32         particleCount = 1024;
    constexpr std::array<f32, 32> rectangle_data = {
        -0.5f, -0.5f, -1.0f, 0.11375187f, 0.44362395f, 0.14269266f, 0.0f, 0.0f,
         0.5f, -0.5f, -1.0f, 0.52293914f, 0.83705337f, 0.41571131f, 0.0f, 0.0f,
        -0.5f,  0.5f, -1.0f, 0.70962868f, 0.64452849f, 0.49889043f, 0.0f, 0.0f,
         0.5f,  0.5f, -1.0f, 0.68304383f, 0.63696862f, 0.48087023f, 0.0f, 0.0f
    };
    constexpr std::array<f32, 32> circle_data = {
        -0.5f, -0.5f, -1.5f, 0, 0, 0, 0.0f, 0.0f,
         0.5f, -0.5f, -1.5f, 0, 0, 0, 0.0f, 1.0f,
        -0.5f,  0.5f, -1.5f, 0, 0, 0, 1.0f, 0.0f,
         0.5f,  0.5f, -1.5f, 0, 0, 0, 1.0f, 1.0f
    };
    VertexArray<false> rect_vao, circle_vao;
    Buffer        rect_vbo, circle_vbo;
    TextureBuffer circle_tex;
    ShaderProgramV2 render;
    
    math::mat4f projMatrix;
    f32 near = 1.0f, far = 100.0f, fov = 65.0f;
    std::vector<math::vec3f> gridPositions;
    std::vector<math::vec4u> gridIndices;

    LoadedImage circle_img;
    LoadedImage::load_image("misc/circle-512.png", true, circle_img);
    // markfmt("Loading Image %u ns", clock.duration());

    contextID = initialize();
    rect_vbo.create({
            (void*)rectangle_data.data(),
            rectangle_data.size(),
            {{ 
                { GL_FLOAT, 3 },
                { GL_FLOAT, 3 },
                { GL_FLOAT, 2 }
            }}
        }, 
        GL_STATIC_DRAW
    );
    circle_vbo.create({
            (void*)circle_data.data(),
            circle_data.size(),
            {{ 
                { GL_FLOAT, 3 },
                { GL_FLOAT, 3 },
                { GL_FLOAT, 2 }
            }}
        }, 
        GL_STATIC_DRAW
    );
    rect_vao.create(rect_vbo);
    circle_vao.create(circle_vbo);


    circle_tex.create({
        { circle_img.x, circle_img.y },
        circle_img.m_data,
        { 4, GL_RGBA, GL_FLOAT, GL_RGBA32F },
        {
            { GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT },
            { GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT },
            { GL_TEXTURE_MIN_FILTER, GL_NEAREST },
            { GL_TEXTURE_MAG_FILTER, GL_NEAREST }
        }
    });

    render.createFrom({
        { "misc/shaders/default/newshader.vert", GL_VERTEX_SHADER   },
        { "misc/shaders/default/newshader.frag", GL_FRAGMENT_SHADER }
    });
    markfmt("Render Shader Status: %u\n", render.compile());


    gl()->Enable(GL_BLEND); 
    gl()->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    gl()->ClearColor(0.0f, 0.5f, 0.7f, 1.0f);
    render.bind();
    

    keepActive = Context::windowActive(contextID); 
    while(keepActive)
    {
        clock.tick();
        begin_frame(); /* Begin & End Frame Are Each-Called on the Active Context! */

        /*
            Everything that needs to happen inside a callback,
            Should be moved to a lambda that can be given to the context
            to execute 
            The lambda will take a struct of all available data 
            inside the callback, and would be written probably in the main,
            as to not pollute the already existing implementation
        */
        auto winSize = Context::windowSize(contextID);
        math::perspective(
            __scast(f32, winSize[0]) / winSize[1], 
            fov, 
            near, 
            far, 
            projMatrix
        );
        render.uniform2ui("WindowSize", winSize);
        render.uniformMatrix4fv("Projection", projMatrix.begin());
        /* 
            End Of Common Data 
        */


        /* Rendering the Simulation Border (Rectangle) */
        render.uniform3f("UseAttribute", { 1.0f, 0.0f, 0.0f });
        rect_vao.bind();
        gl()->DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        /* Rectangle END */


        /* Rendering a Single Particle (Rectangle with texture) */
        render.uniform3f("UseAttribute", { 0.0f, 1.0f, 0.0f });
        render.uniform1i("Texture0", 0);
        circle_vao.bind();
        circle_tex.bindToUnit(0);
        gl()->DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        /* Particle END */

        keyCounter += Input::isKeyPressed(Input::keyCode::W);
        if(Input::isKeyPressed(Input::keyCode::W)) {
            gl()->PolygonMode( GL_FRONT_AND_BACK, (keyCounter & 0x1) == 0 ? GL_FILL : GL_LINE );
        }
        
        /* Happens before end_frame() because Input is reset by then */
        keepActive = Context::windowActive(contextID);
        keepActive = keepActive && !Input::isKeyPressed(Input::keyCode::ESCAPE);
        // keepActive = keepActive && frameCounter < 5;


        end_frame();
        clock.tock();
        ++frameCounter;
        // markfmt("frame took %llu [ms]", clock.duration());
    }


    (void)frameCounter;
    rect_vbo.destroy();
    rect_vao.destroy();
    circle_vbo.destroy();
    circle_vao.destroy();
    circle_tex.destroy();
    LoadedImage::destroy(circle_img);
    render.destroy();
    destroy();
    return 0;
}




u8 initialize()
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
        WindowDescriptor{ {{ 1920u, 1080u }}, nullptr }
    );
    return ctxid;
}




/* 
    This is inefficient - Take all the points on the border
    then,
    go by row,    connect all points top to bottom
    go by column, connect all points left to right
*/
void createGrid(
    u32 width, 
    u32 height, 
    std::vector<math::vec3f>& pos,
    std::vector<math::vec4u>& indices
) {
    pos.resize( (width + 1) * (height + 1) );
    indices.resize( height + width );
    
    
    math::vec3f invDim;
    for(u32 j = 0; j < height; ++j) 
    {
        for(u32 i = 0; i < width; ++i) 
        {
            invDim = {
                1.0f / width,
                1.0f / height,
                0.0f
            };
            pos[i + j * height] = {
                __scast(f32, i),
                __scast(f32, j),
                0.0f
            };
            pos[i + j * height] *= invDim;
        }
    }
    for(u32 j = 0; j < height; ++j) { /* Horizontal lines */
        indices.push_back({
            0, j, width - 1, j
        });
    }
    for(u32 i = 0; i < width; ++i) { /* Vertical Lines */
        indices.push_back({
            i, 0, i, height - 1
        });
    }
    /* Use glDrawElements(GL_LINES, indices.size() * 4, GL_UNSIGNED_INT, NULL) */
    return;
} 


/*

    // compute.createFrom({
    //     { "misc/shaders/default/shader.comp", GL_COMPUTE_SHADER } 
    // });
    // debug_messagefmt("Compute Shader Status: %u\n", compute.compile());
    
    
    // glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);
    
    Loop:
    Bind VertexBuffer As SSBO
    glDispatchCompute()
    glMemoryBarrier()
    Bind VertexBuffer with VAO
    glDrawInstanced(...)
    

*/