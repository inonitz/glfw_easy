#include "compute_rtrace.hpp"
#include "../imgui185/imgui.h"
#include "../Context/context.hpp"
#include "../Context/render.hpp"
#include "../Context/input.hpp"
#include "../Context/window.hpp"
#include "../math/vec.hpp"




#define MONITOR_MAX_WIDTH  1280
#define MONITOR_MAX_HEIGHT 720

#define TEX_WIDTH  640
#define TEX_HEIGHT 360


int compute_raytrace()
{
	std::array<i32, 2> wgpDims = { MONITOR_MAX_WIDTH, MONITOR_MAX_HEIGHT }; // { TEX_WIDTH, TEX_HEIGHT, 1 };


	Context::init(true, MONITOR_MAX_WIDTH, MONITOR_MAX_HEIGHT);
	markstr("AAAAAAAAAAa");
	Renderer::bindGLObjects();
	markstr("AAAAAAAAAAa");
	while(Context::active()) 
	{
		markstr("AAAAAAAAAAa");
		Renderer::beginBackendFrame();
		markstr("AAAAAAAAAAa");

		ImGui::BeginPopup("Program Parameters");
		ImGui::BeginGroup();

		ImGui::Text("%.7f [frame/sec]    \n",  1E+9 / Context::deltaTime());
		ImGui::Text("%.7f [ms/frame]     \n", Context::deltaTime() / 1E+6);
		ImGui::Text("%.7f [delta_avg(ms)]\n", Context::avgDelta()  / 1E+6);

		ImGui::SliderInt("Window Width ", &wgpDims[0], 0, MONITOR_MAX_WIDTH);
		ImGui::SliderInt("Window Height", &wgpDims[1], 0, MONITOR_MAX_HEIGHT);

		ImGui::EndGroup();
		ImGui::EndPopup();


		wgpDims[0] = (wgpDims[0] % 2) + wgpDims[0];
		wgpDims[1] = (wgpDims[1] % 2) + wgpDims[1];
		Renderer::dispatchCompute({ wgpDims[0], wgpDims[1], 1});
		Renderer::waitForCompute();
		Renderer::RenderFBO(wgpDims);
		Renderer::endBackendFrame();
	}


	Context::terminate();
	return 0;
}