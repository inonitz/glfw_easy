#include "raytracewk.hpp"
#include "gl/shader.hpp"
#include "gl/vertexArray.hpp"
#include "gl/textureold.hpp"




int raytracerwk()
{
	auto* context = getGlobalContext();
	defaultCallbacks eventFuncs = {
		glfw_error_callback,
		glfw_framebuffer_size_callback,
		glfw_key_callback,
		glfw_cursor_position_callback,
		glfw_mouse_button_callback
	};
	bool running, focused = true, paused = false, changedResolution = false, refresh[2] = { false, false };
	u32 windowWidth = 1280, windowHeight = 720;
	i32 uniform_samplesppx = 16;
	f32 uniform_randnum    = randnorm32f();
	ComputeGroupSizes computeGroups;

	Program shader, compute;
	VertexArray   vao;
	TextureBuffer tex;
	Buffer vbo, ibo;
	// SceneData data = {
	// 	malloc(sizeof(Sphere) * 16),
	// 	16,
	// 	2,
	// 	CameraTransform{

	// 	}
	// }
	// ShaderStorageBuffer ssbo;
	
	

	/* Init */
    context->glfw.create(windowWidth, windowHeight, eventFuncs);	
	context->frameIndex = 0;
	

	/* Initialize OpenGL stuff */
	debug( /* Enable Advanced Debugging if enabled. */
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_message_callback, nullptr);
		// glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	);
	glEnable(GL_DEPTH_TEST); 
	glClearColor(0.45f, 1.05f, 0.60f, 1.00f);




	computeGroups = recomputeDispatchSize({ windowWidth, windowHeight });
	shader.fromFiles({
		{ "C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/raytrace/shader.vert", GL_VERTEX_SHADER   },
		{ "C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/raytrace/shader.frag", GL_FRAGMENT_SHADER }
	});
	compute.fromFilesCompute({
			{ "C:/Program Files/Programming Utillities/CProjects/mglw-strip/assets/shaders/raytrace/shader.comp", GL_COMPUTE_SHADER },
		},
		computeGroups.localGroup
	);

	ifcrash(!shader.success() || !compute.success());


	vbo.create(BufferDescriptor{ 
			(void*)squareVertices.data(), 4u, 
			{{
				{ GL_FLOAT, 3 },
				{ GL_FLOAT, 2 }
			}}
		},
		GL_STREAM_DRAW
	);
	ibo.create(BufferDescriptor{ 
			(void*)squareIndices.data(), __scast(u32, squareIndices.size()), 
			{{
				{ GL_UNSIGNED_INT, 1 },
			}}
		},
		GL_STREAM_DRAW
	);
	vao.create(vbo, ibo);

	// ssbo.create(BufferDescriptor{
	// 	nullptr,

	// }, GL_DYNAMIC_STORAGE_BIT);


	tex.create(TextureBufferDescriptor
	{
		{ windowWidth, windowHeight },
		nullptr,
		{4, GL_RGBA, GL_FLOAT, GL_RGBA32F },
		{
			{ GL_TEXTURE_WRAP_S, 	 GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_WRAP_T, 	 GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR 		  },
			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR 		  },
		}
	});
	tex.bindToImage(0, TEX_IMAGE_WRITE_ONLY_ACCESS);




	/* Main Loop */
	context->glfw.unlockCursor();
	running = !context->glfw.shouldClose() && !isKeyPressed(KeyCode::ESCAPE);
	while (running)
	{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        context->glfw.procUpcomingEvents();
		if(focused)
		{
			renderImGui(uniform_samplesppx, uniform_randnum, computeGroups.dispatchGroup);
			if(!paused)
			{
				/* Compute Shader Pass Here */
				compute.bind();
				compute.uniform1f("u_dt", uniform_randnum);
				compute.uniform1i("samples_per_pixel", uniform_samplesppx);
				glDispatchCompute(computeGroups.dispatchGroup.x, computeGroups.dispatchGroup.y, computeGroups.dispatchGroup.z);
				glMemoryBarrier(GL_ALL_BARRIER_BITS);
				
				
				/* Vertex Shader Pass Here */
				shader.bind();
				tex.bindToUnit(0);
				shader.uniform1i("tex", 0);


				/* Bind VAO & Draw Here */
				vao.bind();
				glDrawElements(GL_TRIANGLES, squareIndices.size(), GL_UNSIGNED_INT, 0);
			}


			// context->glfw.setCursorMode( !paused);
            if(refresh[0]) {
                shader.reload();
                refresh[0] = shader.success();
            }
            if(refresh[1]) {
                compute.reload();
                refresh[1] = compute.success();
            }
			if(changedResolution)
			{
				windowWidth  = context->glfw.dims[0];
				windowHeight = context->glfw.dims[1];
				tex.recreateImage({ windowWidth, windowHeight });
				tex.bindToUnit(0);
				tex.bindToImage(0, TEX_IMAGE_WRITE_ONLY_ACCESS);
				
				computeGroups = recomputeDispatchSize({ windowWidth, windowHeight });
				compute.reloadCompute(computeGroups.localGroup);
			}
		}


        running = !context->glfw.shouldClose() && !isKeyPressed(KeyCode::ESCAPE);
        focused = !context->glfw.minimized();
        paused  = paused ^ isKeyPressed(KeyCode::P);
		refresh[0] = isKeyPressed(KeyCode::NUM9);
        refresh[1] = isKeyPressed(KeyCode::NUM0);
		changedResolution = context->glfw.windowSizeChanged();


        context->glfw.procOngoingEvents();
		++context->frameIndex;
	}
	context->glfw.close();


	compute.destroy();
	shader.destroy();
	tex.destroy();
	vao.destroy();
	ibo.destroy();
	context->glfw.destroy();
	return 0;
}