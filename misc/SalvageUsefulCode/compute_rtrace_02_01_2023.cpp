#include "compute_rtrace.hpp"
#include "../include/imgui185/imgui.h"
#include "../include/wrapper.hpp"
#include "../include/shader.hpp"
#include "../include/file.hpp"
#include "../include/texture.hpp"
#include "../include/vertexArray.hpp"
#include "../include/math/ray.hpp"
#include "../include/timer.hpp"



#define MONITOR_MAX_WIDTH  1920
#define MONITOR_MAX_HEIGHT 1080

#define TEX_MAX_WIDTH  640
#define TEX_MAX_HEIGHT 360

int compute_raytrace()
{
	Timer<f32>         tmr{};
	Window 			   win{};
	std::array<i32, 2> dims    = { 0, 0    };
	std::array<i32, 3> wgpDims = { 0, 0, 1 };
	i32 	    	   tempVal = 0;
	f32                tmp	   = 0.0f;
	std::string 	   str[4]  = {
		"/home/inon/mglw/assets/shaders/compute_rtrace/shader.vert",
		"/home/inon/mglw/assets/shaders/compute_rtrace/shader.frag",
		"/home/inon/mglw/assets/shaders/compute_rtrace/shader.comp",
		"texData"
	};
	manvec<float> vertices = {
	    // positions          // texture coords
	     1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 1.0f,  // top right
	     1.0f, -1.0f, 0.0f, 1.0f,  1.0f, 0.0f,  // bottom right
	    -1.0f, -1.0f, 0.0f, 1.0f,  0.0f, 0.0f,  // bottom left
	    -1.0f,  1.0f, 0.0f, 1.0f,  0.0f, 1.0f   // top left 
	};
	manvec<u32> indices = {
		0, 1, 3, /* first  triangle */
		1, 2, 3  /* second triangle */
	};




	win.create(MONITOR_MAX_WIDTH, MONITOR_MAX_HEIGHT);
	dims = win.dimensions();


	Shader compute({ loadShader(str[2], ShaderType::Compute ) });
	Shader program({ loadShader(str[0], ShaderType::Vertex  ),
					 loadShader(str[1], ShaderType::Fragment) 
	});
	compute.compile();
	compute.printComputeInfo();
	program.compile();


	VertexBuffer vbo{ vertices.begin(), vertices.bytes(), GL_DYNAMIC_STORAGE_BIT };
	IndexBuffer  ibo{ indices.begin(),  indices.bytes(),  GL_DYNAMIC_STORAGE_BIT };
	VertexArray  vao{
		vbo,
		ibo,
		{
			{ 4, (u32)VertexDataType::f32, GL_FALSE, 0 },
			{ 2, (u32)VertexDataType::f32, GL_FALSE, 0 }
		}
	};
	// vao.debugInfo();


	Texture2D tex{GL_RGBA32F, 1, dims,
		{
			{ GL_TEXTURE_WRAP_S, 	 GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_WRAP_T, 	 GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR 		  },
			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR 		  }
		}
	};


	tex.bindToIndex(0);
	vao.bind();
	while(!win.closed()) 
	{
		tmr.start();
		win.beginFrame();


		compute.setActive();
		{ 
			ImGui::BeginPopup("Program Parameters");
			ImGui::BeginGroup();

			ImGui::Text("%.7f ms/frame\n", 1000.0f / ImGui::GetIO().Framerate);
			ImGui::Text("%.7f ms/frame\n", tmp / 1E+6f);

			ImGui::SliderInt("Window Width ", &dims[0], 0, TEX_MAX_WIDTH );
			ImGui::SliderInt("Window Height", &dims[1], 0, TEX_MAX_HEIGHT);

			ImGui::EndGroup();
			ImGui::EndPopup();
		}
		wgpDims = { dims[0], dims[1], 1 };
		compute.beginCompute(wgpDims);
		compute.barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		

		program.setActive();
		program.setUniform("texData", UniformType::i32p, 1, &tempVal);
		tex.activeTexture(0);
		glDrawElements(GL_TRIANGLES, indices.len(), vdtypeToGL(VertexDataType::u32), nullptr);

		win.endFrame();
		tmp = tmr.end();
	}


	/* Cleanup */
	vbo.destroy();
	ibo.destroy();
	vao.destroy();

	compute.destroy();
	program.destroy();
	win.destroy();

	return 0;
}