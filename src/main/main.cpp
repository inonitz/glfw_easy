#include "awc2fluid/simple.hpp"
#include "gpu-gems38/gem38.hpp"
#include "util/marker2.hpp"


#define run_legacy_code 1


int main() {
    i32 out = (run_legacy_code == 1) ? render_fluid_awc2_fuckyou() : render_gpugems38();
    markstr("Successful Exit");
    return out;
}


/* 
	Useful code (?)
*/
// void renderImGui(globalContext* const ctx, math::mat4f const& modelMatrix)
// {
// 	const math::mat4f* print = nullptr;
// 	const f32          dt 	 = ctx->glfw.time_dt();
// 
// 	/*
// 		ImGui Input Capture & rendering
// 	*/
// 	ImGui::BeginGroup();
// 	
// 	// ImGui::BeginGroup();
// 	// ImGui::SliderInt("WorkGroup [x]", &workGroupSizeTest[0], 1, windowWidth);
// 	// ImGui::SliderInt("WorkGroup [y]", &workGroupSizeTest[1], 1, windowHeight);
// 	// ImGui::EndGroup();
// 
// 	ImGui::SliderFloat("Field Of View      ", &ctx->persp.__.fieldOfView, 20.0f, 200.0f);
// 	ImGui::SliderFloat("Near Clip Plane    ", &ctx->persp.__.nearClip, 0.01f, 50.0f);
// 	ImGui::SliderFloat("Far  Clip Plane    ", &ctx->persp.__.farClip, 50.0f, 1000.0f);
// 	ImGui::SliderFloat("Camera Speed       ", &ctx->cam.velocity.u, 1.0f, 50.0f);
// 	ImGui::SliderFloat("Camera Rotate Speed", &ctx->cam.velocity.v, 0.01, 7.5f);
// 	// ctx->persp.__.aspectRatio = ctx->glfw.aspectRatio<f32>();
// 	ImGui::EndGroup();
// 
// 
// 	ImGui::BeginGroup();
// 	print = ctx->persp.constptr();
// 	ImGui::Text("Rendering at %.02f Frames Per Second (%.05f ms/frame)", (1.0f / dt), (dt * 1000.0f) );
// 	ImGui::Text("Proj:\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n\n", 
// 		print->m00, print->m01, print->m02, print->m03,
// 		print->m10, print->m11, print->m12, print->m13,
// 		print->m20, print->m21, print->m22, print->m23,
// 		print->m30, print->m31, print->m32, print->m33
// 	);
// 	print = ctx->cam.constptr();
// 	ImGui::Text("View:\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n\n", 
// 		print->m00, print->m01, print->m02, print->m03,
// 		print->m10, print->m11, print->m12, print->m13,
// 		print->m20, print->m21, print->m22, print->m23,
// 		print->m30, print->m31, print->m32, print->m33
// 	);
// 	print = &modelMatrix;
// 	ImGui::Text("Model:\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n( %+.05f, %+.05f, %+.05f, %+.05f )\n\n", 
// 		print->m00, print->m01, print->m02, print->m03,
// 		print->m10, print->m11, print->m12, print->m13,
// 		print->m20, print->m21, print->m22, print->m23,
// 		print->m30, print->m31, print->m32, print->m33
// 	);
// 	ImGui::EndGroup();
// }
// 
// 
// void glbinding_init()
// {
//     glbinding::Binding::initialize([](const char * name) {
//         return glfwGetProcAddress(name);
//     }, false);
// }
// 
// 
// void glbinding_error(bool enable)
// {
//     if (enable)
//     {
//         glbinding::Binding::setCallbackMaskExcept(glbinding::CallbackMask::After, { "glGetError" });
// 
//         glbinding::Binding::setAfterCallback([](const glbinding::FunctionCall &)
//         {
//             gl::GLenum error = gl::glGetError();
//             if (error != gl::GL_NO_ERROR)
//                 std::cout << "Error: " << error << std::endl;
//         });
//     }
//     else
//         glbinding::Binding::setCallbackMask(glbinding::CallbackMask::None);
// }
// 
// 
//