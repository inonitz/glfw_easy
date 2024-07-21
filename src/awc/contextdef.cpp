#include "contextdef.hpp"
#include "ImGui/imgui.h"
#include "awc/event.hpp"
#include "def_callback.hpp"
#include "event.hpp"
#include "opengl2.hpp"
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/imgui_impl_glfw.h>
#include <cstdio>
#include "util/marker2.hpp"
#include "util/types.hpp"
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


    ctx.unit->reset();
    *ctx.callbacks = (override_funcs.pointers[0] == 0) ? 
        AWC::Event::defaultCallbacks : 
        override_funcs;
    util::__memset<u64>(ctx.usercallbacks->pointers, 
        ( sizeof(ctx.usercallbacks->pointers) / sizeof(u64) ), 
        __rcast(u64, &user_callback_func_noop)
    );

    /* Window Init */
    ctx.win->create(desc, options.bits);
    ctx.win->setCurrent();
    ctx.win->setEventHooks(ctx.callbacks);

    /* OpenGL Init after glfw */
    Context::OpenGL::create(context_id);

    /* Init ImGui Context and Related Backends - in this case GLFW & OpenGL Backends */
    ImGui::SetCurrentContext(ctx.imgui);
    ImGui::StyleColorsDark();
    glver = ImGui_ImplGlfw_InitForOpenGL(ctx.win->underlying_handle(), false);
    glver = glver && ImGui_ImplOpenGL3_Init("#version 460");
    if(!glver) {
        markstr("AWC::Context::init(...) => Couldn't initialize ImGui's OpenGL & GLFW State\n");
    }
    return glver;
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
    util::__memset(ctx.callbacks,     1);
    util::__memset(ctx.usercallbacks, 1);
    ctx.unit->reset();
    ctx.win->destroy();
    return;
}


} // namespace AWC