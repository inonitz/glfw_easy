#include "context.hpp"
#include "contextdef.hpp"
#include "instance.hpp"
#include "macro.hpp"
#include "input.hpp"
#include "state.hpp"
#include "util/ifcrash.hpp"
#include "util/marker2.hpp"
#include <glbinding/glbinding.h>
#include <glbinding/gl/gl.h>
#include <GLFW/glfw3.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>


namespace AWC::Context {


u8 allocate()
{
    auto* glibinst = __get_instance();
    ifcrashfmt_debug( __awc_lib_context_count() == (AWC_LIB_CONTEXT_MAX),
        "AWC::Context::allocate() => Maximum amunt of Contexts allocated%c", '\n'
    );
    auto* galloc = &glibinst->mempool;
    AWCContext newctxt = {
        __rcast(WindowContext*, galloc->windows.allocate() ),
        __rcast(Input::InputUnit*, galloc->inputs.allocate() ),
        __rcast(Event::callbackTable*, galloc->handler_tables.allocate() ),
        __rcast(Event::userCallbackTable*, galloc->userhandler_tables.allocate() ),
        ImGui::CreateContext()
    };


    if(newctxt.imgui == nullptr 
        || newctxt.usercallbacks == nullptr 
        || newctxt.callbacks == nullptr 
        || newctxt.unit == nullptr 
        || newctxt.win == nullptr
    ) {
        markstr("AWC::Context::allocate() => Failed To Create Context\n");
        markfmt("\
AWC::Context::allocate() => Failed To Create Context\n\
WindowContext* %llx\n\
Input::InputUnit* %llx\n\
Event::callbackTable* %llx\n\
Event::userCallbackTable* %llx\n\
ImGui::CreateContext() %llx\n",
            newctxt.win, newctxt.unit,
            newctxt.callbacks, newctxt.usercallbacks,
            newctxt.imgui
        );
        return 0;
    }


    u8 newCount = __awc_lib_context_count();
    glibinst->contexts[newCount] = newctxt;


    AWC_LIB_MODIFY_VAR_BITS(glibinst->flags, 
        AWC_LIB_CONTEXT_COUNT_MASK, 
        ++newCount << AWC_LIB_CONTEXT_COUNT_SHIFT
    );
    return newCount;
}


bool init(
    u8                               context_id,
    AWC::WindowOptions        const& options,
    AWC::WindowDescriptor     const& desc,
    AWC::Event::callbackTable const& override_funcs
) {
    AWC::AWCContext::create(*__get_context(context_id), context_id,
        options,
        desc,
        override_funcs
    );
    /* Set Library Flag */
    AWC_LIB_MODIFY_VAR_BITS(__get_instance()->flags, 
        AWC_LIB_ATLEAST_ONE_CONTEXT_MASK, 
        AWC_LIB_ATLEAST_ONE_CONTEXT_MASK
    );
    return 1;
}


void setActive(u8 id)
{
    /* Consistency Check for developer */
    ifcrashfmt_debug( __awc_lib_context_count() < id,
        "AWC::Context::setActive() => ID %u exceeds Currently Allocated Context Amount (%u)", 
        id, AWC_LIB_CONTEXT_MAX
    );


    /* Set Library Flag */
    AWC_LIB_MODIFY_VAR_BITS(__get_instance()->flags, 
        AWC_LIB_ACTIVE_CONTEXT_MASK, 
        (id << AWC_LIB_ACTIVE_CONTEXT_SHIFT)
    );
    glfwMakeContextCurrent(__active_context()->win->underlying_handle());
    glbinding::useContext(__awc_lib_active_context());
    ImGui::SetCurrentContext(__active_context()->imgui);
    gl::glClearColor(0.0f, 1.0f, __scast(f32, __awc_lib_active_context()) / __awc_lib_context_count(), 1.0f);
    return;
}


void begin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    return;
}


void end()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    __active_context()->win->swapBuffers();
    __active_context()->unit->reset();
    return;
}


bool isActive(u8 id) {
    /* can use lib flags instead of glfw, since we track state on context changes in AWCData */
    return __awc_lib_active_context() == id;
}

bool shouldClose(u8 id) {
    return __get_context(id)->win->shouldClose();
}


template<typename T> std::array<T, 2> windowSize(u8 id) 
{
    std::array<u32, 2> size = { 0, 0 };
    memcpy(
        __scast(void*, size.data()), 
        __scast(void*, __get_context(id)->win->getSize() ),
        2 * sizeof(u32)
    );
    return std::array<T, 2> { __scast(T, size[0]), __scast(T, size[1]) };
}


template std::array<u32, 2> windowSize<u32>(u8 id);
template std::array<i32, 2> windowSize<i32>(u8 id);


} // namespace AWC::Context