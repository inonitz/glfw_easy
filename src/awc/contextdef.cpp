#include "contextdef.hpp"
#include "def_callback.hpp"
#include "opengl2.hpp"
#include <Imgui/imgui_impl_opengl3.h>
#include <ImGui/imgui_impl_glfw.h>
#include "util/marker2.hpp"
#include "util/util.hpp"


namespace AWC {


bool AWCContext::create(
    AWCContext& ctx,
    u8                               context_id,
    AWC::WindowOptions        const& options,
    AWC::WindowDescriptor     const& desc,
    AWC::Event::callbackTable const& override_funcs
) {
    i32 glver = 1;

    /* Input Buffer reset */
    ctx.unit->reset();
    
    /* Callback Function Table Reset/Init */
    *ctx.callbacks = (override_funcs.pointers[0] == 0) ? 
        AWC::Event::defaultCallbacks 
        : 
        override_funcs;
    for(auto& ptr : ctx.usercallbacks->pointers) {
        ptr = __rcast(u64, &user_callback_func_noop);
    }

    /* Window Init */
    ctx.win->create(desc, options.bits);
    ctx.win->setCurrent();
    ctx.win->setEventHooks(ctx.callbacks);


    /* OpenGL Init after glfw */
    Context::OpenGL::create(context_id);


    /* Init ImGui Context and Related Backends - in this case GLFW & OpenGL Backends */
    IMGUI_CHECKVERSION();
    ImGui::SetCurrentContext(ctx.imgui);
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(ctx.win->underlying_handle(), false);
    glver = ImGui_ImplOpenGL3_Init("#version 460");
    if(!glver) {
        markstr("AWC::Context::init(...) => Couldn't initialize ImGui's OpenGL Context\n");
        return 0;
    };


    return 1;
}


void AWCContext::destroy(AWCContext& ctx, u8 context_id) {
    /* Shutdown ImGui Related stuff */
    ImGui::SetCurrentContext(ctx.imgui);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(ctx.imgui);

    /* Destroy OpenGL State */
    Context::OpenGL::destroy(context_id);


    /* Reset/Destroy Event Handlers, Input Buffers, and Window System (GLFW) */
    util::__memset<byte>(__rcast(byte*, ctx.callbacks),     sizeof(Event::callbackTable),     0x00);
    util::__memset<byte>(__rcast(byte*, ctx.usercallbacks), sizeof(Event::userCallbackTable), 0x00);
    ctx.unit->reset();
    ctx.win->destroy();
    return;
}


} // namespace AWC