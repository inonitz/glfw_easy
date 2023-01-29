#include "basicpp.hpp"
#include <stb_image/stb_image.hpp>
#include <ImGui/imgui.h>
#include <glad/glad.h>
#include "../context.hpp"
#include "../file.hpp"
#include <immintrin.h>
#include <utility>



void loadShaders(std::array<u32, 2>& shaders, u32& shaderProgram);
void convertRGBA_U32_TO_F32(u8* inData, float* outData, i32 dimx, i32 dimy);
void recreateImage();




int basicpp() {
	auto* context = getGlobalContext();
	u32 			 windowWidth  = 1920;
	u32 			 windowHeight = 1080;
	defaultCallbacks eventFuncs = {
		glfw_error_callback,
		glfw_framebuffer_size_callback,
		glfw_key_callback,
		glfw_cursor_position_callback,
		glfw_mouse_button_callback
	};


	std::array<u32, 2> shaders;
	u32 shaderProgram;
	u32 VAO, VBO, EBO, TEX;
	std::array<float, 4> windowColor  		  = { 0.45f, 0.55f, 0.60f, 1.00f };
	std::array<float, 4> texelColorMultiplier = { 1.0f,   0.0f, 0.5f,   1.0f };
	

	math::mat4f model;
	math::vec3f cameraPosition = { 0.0f, 0.0f, -3.0f };
	f32 fieldOfView   = 90.0f;
	f32 aspectRatio   = windowWidth / (f32)windowHeight;
	f32 nearClipPlane = 0.1f;
	f32 farClipPlane  = 100.0f;




	context->glfw.create(windowWidth, windowHeight, eventFuncs);
	context->camera.create({ fieldOfView, aspectRatio, nearClipPlane, farClipPlane }, cameraPosition);

	/* Initialize OpenGL stuff */
	debug( /* Enable Advanced Debugging if enabled. */
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_message_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	);
	glEnable(GL_DEPTH_TEST); 
	glClearColor(windowColor[0], windowColor[1], windowColor[2], windowColor[3]);


	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);
	glCreateBuffers(1, &EBO);

	glNamedBufferStorage(VBO, vertices.size() * sizeof(f32), vertices.begin().base(), GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferStorage(EBO, indices.size()  * sizeof(u32), indices.begin().base(),  GL_DYNAMIC_STORAGE_BIT);


	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(VAO, EBO);

	glEnableVertexArrayAttrib(VAO, 0);
	glEnableVertexArrayAttrib(VAO, 1);
	glVertexArrayAttribFormat(VAO, 0, 4, GL_FLOAT, GL_FALSE, 0              );
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32));
	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribBinding(VAO, 1, 0);


	stbi_set_flip_vertically_on_load(true);
	i32 TEXW, TEXH, channels, desiredChannels = 4;
	u8*  textureBuffer 		  = stbi_load(paths[3], &TEXW, &TEXH, &channels, desiredChannels);
	f32* textureBufferAsFloat = amalloc_t(f32, (size_t)TEXW * TEXH * sizeof(f32) * desiredChannels, CACHE_LINE_BYTES);
	
	convertRGBA_U32_TO_F32(textureBuffer, textureBufferAsFloat, TEXW, TEXH);
	stbi_image_free(textureBuffer);
	
	glCreateTextures(GL_TEXTURE_2D, 1, &TEX);
	glBindTexture(GL_TEXTURE_2D, TEX);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		TEXW,
		TEXH,
		0,
		GL_RGBA,
		GL_FLOAT,
		textureBufferAsFloat	
	);
	glBindTexture(GL_TEXTURE_2D, 0);
	afree_t(textureBufferAsFloat);
	glTextureParameteri(TEX, GL_TEXTURE_WRAP_S, 	 GL_REPEAT 			   );			 
	glTextureParameteri(TEX, GL_TEXTURE_WRAP_T, 	 GL_REPEAT 			   );		 
	glTextureParameteri(TEX, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(TEX, GL_TEXTURE_MAG_FILTER, GL_LINEAR 			   );
	glGenerateTextureMipmap(TEX);
	loadShaders(shaders, shaderProgram);


	const u32 uniformColorLocation = glGetUniformLocation(shaderProgram, "texelMultiply");
	const u32 uniformModelMatrix   = glGetUniformLocation(shaderProgram, "model");
	const u32 uniformViewMatrix    = glGetUniformLocation(shaderProgram, "view");
	const u32 uniformProjMatrix    = glGetUniformLocation(shaderProgram, "projection");

	glBindVertexArray(VAO);
	glUseProgram(shaderProgram);
	glBindTextureUnit(1, TEX);
	glUniform1i(glGetUniformLocation(shaderProgram, "texData"), 1);
	glUniform4f(
		uniformColorLocation,  
		texelColorMultiplier[0], 
		texelColorMultiplier[1], 
		texelColorMultiplier[2], 
		texelColorMultiplier[3]
	);


	math::identity(model);
	math::mat4f tmp0, tmp1;
	while(!context->glfw.shouldClose())
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		context->glfw.procUpcomingEvents();

		ImGui::SliderFloat4("Texel Color Multiplier", texelColorMultiplier.begin(), 0.0f, 1.0f);
		ImGui::BeginGroup();
		ImGui::SliderFloat("Field Of View  ", &fieldOfView, 45.0f, 130.0f);
		ImGui::SliderFloat("Near Clip Plane", &nearClipPlane, 0.1f, 10.0f);
		ImGui::SliderFloat("Far  Clip Plane", &farClipPlane, 50.0f, 200.0f);
		aspectRatio = context->glfw.aspectRatio<f32>();
		ImGui::EndGroup();
		
	 	context->camera.updateProjectionArguments(fieldOfView, aspectRatio, nearClipPlane, farClipPlane);
	

		glUniform4f(
			uniformColorLocation,  
			texelColorMultiplier[0], 
			texelColorMultiplier[1], 
			texelColorMultiplier[2], 
			texelColorMultiplier[3]
		);
		glUniformMatrix4fv(uniformModelMatrix, 1, false, model.begin()	   		        );
		glUniformMatrix4fv(uniformViewMatrix,  1, false, context->camera.view().begin());
		glUniformMatrix4fv(uniformProjMatrix,  1, false, context->camera.projection().begin());

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
		context->glfw.procOngoingEvents();

		run_block(5, {
			tmp0 = context->camera.view();
			math::inverse(tmp0, tmp1);
			tmp1 = context->camera.inverse_view();
			math::MultiplyMat4Mat4(tmp0, tmp1, model);
			printf("--------------\n");
			tmp0.print();
			tmp1.print();
			model.print();
			printf("--------------\n");
		});
		math::identity(model);
	}

	
	glDeleteProgram(shaderProgram);
	glDeleteTextures(1, &TEX);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	context->glfw.destroy();
	return 0;
}




void loadShaders(std::array<u32, 2>& shaders, u32& shaderProgram)
{
	debugnobr(
	    static char genericErrorLog[1024];
	);
	size_t size   		  = 0;
	char*  src    		  = nullptr;
	i32    length 		  = 0;
	i32    successStatus  = 0;
	u32    shaderTypes[2] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };


	shaderProgram = glCreateProgram();
	for(u32  i = 0; i < sizeof(shaders) / sizeof(u32); ++i)
	{
		shaders[i] = glCreateShader(shaderTypes[i]);

		size = 0;
		src  = nullptr;

		loadFile(paths[i], &size, src);
		src    = amalloc_t(char, size, CACHE_LINE_BYTES);
		loadFile(paths[i], &size, src);
		length = static_cast<i32>(size);


		glShaderSource(shaders[i], 1, (const char**)&src, &length);
		glCompileShader(shaders[i]);
		glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &successStatus);
		ifcrashdo(!successStatus, { /* Safety Check. More info in debug mode. */
			glGetShaderInfoLog(shaders[i], sizeof(genericErrorLog), &length, genericErrorLog);
        	printf("Shader [type %u] Error Log[%u Bytes]: %s\n", shaderTypes[i], length, genericErrorLog);
    	});
		afree_t(src);

		glAttachShader(shaderProgram, shaders[i]);
	}


	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &successStatus);
	ifcrashdo(!successStatus, {
		glGetProgramInfoLog(shaderProgram, sizeof(genericErrorLog), NULL, genericErrorLog);
		printf("%s\n", genericErrorLog);
	});


	glDeleteShader(shaders[0]);
	glDeleteShader(shaders[1]);
	return;
}






void convertRGBA_U32_TO_F32(u8* inData, float* outData, i32 dimx, i32 dimy)
{
    size_t currPixel = 0;
    size_t bufLength = 4 * static_cast<size_t>(dimx) * dimy;

#ifdef __AVX2__
    markstr("AVX2\n");
    ifcrashdo( ( (size_t)outData & (8 * sizeof(float) - 1) ) != 0, {
        printf("outData isn't aligned on (atleast) a 32 Byte memory boundary\n");
    });
	__m256  constf32;
	__m128i A; 
	__m256i B;
	__m256  C;
	__m256  D;
	constf32 = _mm256_broadcastss_ps(_mm_set_ss(0.00392156862745098f)); /* load constant 1/255 to each element */
    // for(i32 c = 0; c < 4; ++c) 
	// {
	// 	for(i32 y = 0; y < dimy; ++y) 
	// 	{
	// 		for(i32 x = 0; x < dimx; x += 8) 
	// 		{
	// 			A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* Load 16 bytes 				       */
	// 			B = _mm256_cvtepu8_epi32 (A); 	     	                   /* Convert first 8 bytes to u32's       */
	// 			C = _mm256_cvtepi32_ps(B);   	     	                   /* Convert u32's to f32's 		       */
	// 			D = _mm256_mul_ps(C, constf32);                            /* Multiply f32's by (1/255). 		   */
	// 		    _mm256_store_ps(&outData[currPixel], D);                   /* Store result in appropriate location */
	// 			currPixel += 8;
	// 		}
	// 	}  
	// }
    for(currPixel = 0; currPixel < bufLength; currPixel += 8) {
        A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* Load 16 bytes 				       */
        B = _mm256_cvtepu8_epi32 (A); 	     	                   /* Convert first 8 bytes to u32's       */
        C = _mm256_cvtepi32_ps(B);   	     	                   /* Convert u32's to f32's 		       */
        D = _mm256_mul_ps(C, constf32);                            /* Multiply f32's by (1/255). 		   */
        _mm256_store_ps(&outData[currPixel], D);                   /* Store result in appropriate location */
    }
#elif defined __SSE4_1__
    markstr("SSE4.1\n");
    ifcrashdo( ( (size_t)outData & (4 * sizeof(float) - 1) ) != 0, {
        printf("outData isn't aligned on (atleast) a 16 Byte memory boundary\n");
    });
    alignsz(16) std::array<f32, 4> const128 = { 0.00392156862745098f, 0.00392156862745098f, 0.00392156862745098f, 0.00392156862745098f };
    __m128 cf32;
    __m128i A, B;
	__m128  C, D;


    cf32 = _mm_load_ps(const128.begin()); /* SSE */
    // for(i32 c = 0; c < 4; ++c) 
	// {
	// 	for(i32 y = 0; y < dimy; ++y) {
    //         for(i32 x = 0; x < dimx; x += 4) 
	// 		{
    //             A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* SSE3 */
    //             B = _mm_cvtepu8_epi32(A); /* SSE4.1 */
    //             C = _mm_cvtepi32_ps(B);   /* SSE2   */
    //             D = _mm_mul_ps(C, cf32);  /* SSE    */
    //             currPixel += 4;
    //         }
    //     }
    // }
    for(currPixel = 0; currPixel < bufLength; currPixel += 4) {
        A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* SSE3 */
        B = _mm_cvtepu8_epi32(A); /* SSE4.1 */
        C = _mm_cvtepi32_ps(B);   /* SSE2   */
        D = _mm_mul_ps(C, cf32);  /* SSE    */
        _mm_store_ps(&outData[currPixel], D);
    }
#else
    markstr("FALLBACK (COMPILER_OPTIMIZE march=native, mtune=native\n");
    alignsz(16) std::array<f32, 4> tmpf{};
    alignsz(16) std::array<u32, 4> tmpu{};
    u32   tmp = 0;


    // for(i32 c = 0; c < 4; ++c) 
	// {   
    //     for(i32 y = 0; y < dimy; ++y) {
    //         for(i32 x = 0; x < dimx; x += 4) {
    //             memcpy(&tmp, &inData[currPixel], 4);
    //             tmpu = {
    //                 tmp & 0x000000FF, /* First  Byte */
    //                 tmp & 0x0000FF00, /* Second Byte */
    //                 tmp & 0x00FF0000, /* Third  Byte */
    //                 tmp & 0xFF000000  /* Fourth Byte */
    //             };
    //             tmpf = {
    //                 static_cast<f32>(tmpu[0]),
    //                 static_cast<f32>(tmpu[1]),
    //                 static_cast<f32>(tmpu[2]),
    //                 static_cast<f32>(tmpu[3]),
    //             };
    //             memcpy(&outData[currPixel], tmpf.begin(), sizeof(f32) * 4);

    //             currPixel += 4;
    //         }
    //     }
    // }
    for(currPixel = 0; currPixel < bufLength; currPixel += 4) 
    {
        memcpy(&tmp, &inData[currPixel], 4);
        tmpu = {
            tmp & 0x000000FF, /* First  Byte */
            tmp & 0x0000FF00, /* Second Byte */
            tmp & 0x00FF0000, /* Third  Byte */
            tmp & 0xFF000000  /* Fourth Byte */
        };
        tmpf = {
            static_cast<f32>(tmpu[0]),
            static_cast<f32>(tmpu[1]),
            static_cast<f32>(tmpu[2]),
            static_cast<f32>(tmpu[3]),
        };
        memcpy(&outData[currPixel], tmpf.begin(), sizeof(f32) * 4);
    }
#endif

    return;
}



/* 
	Current Goals:
	when a repeat is detected, make sure to save the state until key is released <= will allow for smooth movement
	math::inv_lookAt() Not working as intended. FIX
	get the texture to appear on screen (more like try to find it renders like it does now lol)

	then try =>
		rendering the grid, with each cube being referencing its own 'particle data'
		compute shader


	later goals:
		render cube
		expand grid to 3D
		instanced rendering? (a lot of draw calls is no good)
*/





