#include <glad/glad.h>
#include "sprite_render.hpp"
#include <stb_image/stb_image.hpp>
#include "util/convert_tex.hpp"




int sprite_render()
{
    auto* ctx = createDefaultContext({ 1280, 720 });
    bool running = true, paused = false, changedResolution = false, refreshShader = false;
    math::vec2u currDims; 

    RectangleData rectObject;
    const Renderer::RenderTarget target = {
        &rectObject.rectVAO,
        &rectObject.texture,
        &rectObject.transform
    };
    createRectangleData(&rectObject);


    ctx->renderer.push(&target);
    while(running)
    {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ctx->glfw.processPreviousFrame();


        if(!paused) {
            if(refreshShader) {
                ctx->shader.refreshShaderSource(0);
                ctx->shader.refreshShaderSource(1);
                running &= ctx->shader.compile();
            }
            /* Game loop stuff here */
            ctx->renderer.renderImGui();
            ctx->renderer.render();
        }

        if(changedResolution) {
            /* ? */
        }

        
        
        running = !ctx->glfw.shouldClose() && !isKeyPressed(Input::KeyCode::ESCAPE);
        paused ^= isKeyPressed(Input::KeyCode::P);
        paused = ctx->glfw.minimized() ? true : paused;
        refreshShader = isKeyPressed(Input::KeyCode::R);
        changedResolution = ctx->glfw.windowSizeChanged();
        
        ctx->glfw.processCurrentFrame();
        ++ctx->frameIndex;
    }
    destroyRectangleData(&rectObject);
    destroyDefaultContext();
    return 0;
}




globalContext* createDefaultContext(math::vec2u windowSize)
{
    auto* ctx = getGlobalContext();


    ctx->glfw.create(windowSize.x, windowSize.y);
    ctx->frameIndex = 0;
	debug( /* Enable Advanced OpenGL Debugging if enabled. */
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_message_callback, nullptr);
		// glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	);
	glEnable(GL_DEPTH_TEST); 
	glClearColor(0.45f, 1.05f, 0.60f, 1.00f);
    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );


    ctx->shader.createFrom({
        // { "C://Program Files//Programming Utillities//CProjects//physics2d//assets//shaders//default//shader.vert", GL_VERTEX_SHADER   },
        // { "C://Program Files//Programming Utillities//CProjects//physics2d//assets//shaders//default//shader.frag", GL_FRAGMENT_SHADER }
        { "C://CTools//Projects//physics2d//assets//shaders//default/shader.vert", GL_VERTEX_SHADER   },
        { "C://CTools//Projects//physics2d//assets//shaders//default/shader.frag", GL_FRAGMENT_SHADER }
        
    });
    ifcrash(ctx->shader.compile() == GL_FALSE);
    ctx->renderer.create(0.0f, __scast(f32, ctx->glfw.dims[0]), 0.0f, __scast(f32, ctx->glfw.dims[1]), -1.0f, 1.0f);

    return ctx;
}


void createRectangleData(RectangleData* rectData)
{
    // constexpr std::array<f32, 16> vertices = {
    //   	-0.75f, -0.75f, 0.0f, 0.0f,
	// 	-0.75f,  0.75f, 0.0f, 1.0f,
	// 	 0.75f,  0.75f, 1.0f, 1.0f,
	// 	 0.75f, -0.75f, 1.0f, 0.0f,
    // };
    constexpr std::array<f32, 16> vertices = {
      	-20.0f, -20.0f,  0.0f, 0.0f,
		-20.0f,  20.0f,  0.0f, 1.0f,
		 20.0f,  20.0f,  1.0f, 1.0f,
		 20.0f, -20.0f,  1.0f, 0.0f,
    };
    constexpr std::array<u32, 6> indices = {
        0, 2, 1,
		0, 3, 2
    };


    // const char* texturePath = "C://Program Files//Programming Utillities//CProjects//physics2d//assets//circle-512.png";
    const char* texturePath = "C://CTools//Projects//physics2d//assets//circle-512.png";
    i32 TEXW, TEXH, TEXCHANNEL;
    u8*  textureBuffer;
    f32* textureBufferAsFloat;
    stbi_set_flip_vertically_on_load(true);
	textureBuffer 		 = stbi_load(texturePath, &TEXW, &TEXH, &TEXCHANNEL, 4);
	textureBufferAsFloat = amalloc_t(f32, (size_t)TEXW * TEXH * sizeof(f32) * 4, CACHE_LINE_BYTES);
	convertRGBA_U32_TO_F32(textureBuffer, textureBufferAsFloat, TEXW, TEXH);
	stbi_image_free(textureBuffer);


    rectData->texture.create(TextureBufferDescriptor{
		{ __scast(u32, TEXW), __scast(u32, TEXH) },
		textureBufferAsFloat,
		{ 4, GL_RGBA, GL_FLOAT, GL_RGBA32F },
		{
			{ GL_TEXTURE_WRAP_S, 	 GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_WRAP_T, 	 GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR 		  },
			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR 		  },
		}
	});


    rectData->vertex.create(BufferDescriptor{ (void*)vertices.data(), 4u, {{
				{ GL_FLOAT, 4 },
			}}
		},
		GL_STATIC_DRAW
	);
	rectData->indices.create(BufferDescriptor{ (void*)indices.data(), __scast(u32, indices.size()), {{
				{ GL_UNSIGNED_INT, 1 },
			}}
		},
		GL_STATIC_DRAW
	);
    rectData->rectVAO.create(rectData->vertex, rectData->indices);
    rectData->transform = {
        math::vec2f{0.0f, 0.0f},
        math::vec2f{0.02f},
        0.0f,
        {}
    };
    math::modelMatrix2d(
        rectData->transform.position,
        rectData->transform.scale,
        rectData->transform.rotateAngle,
        rectData->transform.TRS
    );


    afree_t(textureBufferAsFloat);
    return;
}


void destroyRectangleData(RectangleData* rectData)
{
    rectData->texture.destroy();
    rectData->vertex.destroy();
    rectData->indices.destroy();
    rectData->rectVAO.destroy();
    return;
}


void destroyDefaultContext()
{
    auto* ctx = getGlobalContext();
    ctx->renderer.destroy();
    ctx->shader.destroy();
    ctx->glfw.destroy();
    return;
}



// using PhysicsHandle = imut_type_handle<struct PhysicsTarget>;
// struct Entity
// {
//     Renderer::RenderHandle rdesc;
//     PhysicsHandle pdesc;
// };




/*
MultiDrawIndirect - batching Multiple draw calls (per object)
Texture Arrays - batching/packing multiple 2D Textures into the same object


const float c_infinity = 1. / 0.;
const float c_pi         = 3.14159265358979323846264338327950288;


float toRadians(float degrees) { return degrees * c_pi / 180.0f; }
float random(in vec2 p)
{
    const vec2 r = vec2(23.1406926327792690, 2.6651441426902251);
    return fract( cos( mod( 123456789., 1e-7 + 256. * dot(p,r) ) ) );  
}

float uintToNormFloat(uint m) { return uintBitsToFloat((m & 0x007FFFFFu) | 0x3F800000u ) - 1.0f; }
float random_norm(in vec2 p)  { return uintToNormFloat(floatBitsToUint(random(p))); 			 }
vec3 simpleRandUnitSphere(in vec2 texelCoord, uint sampleNum)
{
    vec3 r3 = vec3(
        random(  vec2(u_rand, random(texelCoord)       )),
        random(  vec2(u_rand, random(vec2(sampleNum))  )), 
        0
    );
    r3.z = random(r3.xy);
    return normalize(r3);
}
*/