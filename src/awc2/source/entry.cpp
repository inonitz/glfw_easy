#include "include/entry.hpp"
#include "internal_instance.hpp"
#include "internal_state.hpp"
#include <GLFW/glfw3.h>
#include <ImGui/imgui.h>
#include "util/marker2.hpp"
#include "util/ifcrash.hpp"
#include "util/aligned_malloc.hpp"
#include "util/util.hpp"


namespace AWC2 {


void init()
{
    auto& __libdata = *internal::__awc2_lib_get_instance();
    
    /* GLFW init */
    glfwSetErrorCallback([](
        __release_unused int errnum, 
        __release_unused const char* errmsg
    ) {
        markfmt("GLFW ERROR %u => %s\n", errnum, errmsg);
    });
    ifcrashstr(!glfwInit(), "AWC2::init() Failed => glfwInit() returned 0");

    /* ImGui init */
    IMGUI_CHECKVERSION();

    /* AWC2 init */
    /* Allocate data for maximal amount of contexts */
    static constexpr u64 ctxsize = sizeof(internal::AWC2ContextData);
    __libdata.poolmem.create(
        util::aligned_malloc<ctxsize>(AWC2::k_maximumContexts * ctxsize),
        AWC2::k_maximumContexts
    );
    util::__memset(__libdata.poolmem.data(), AWC2::k_maximumContexts);

    /* Init memory-manager (pool-alloc) */
    __libdata.ctxpool.create(__libdata.poolmem.data(), AWC2::k_maximumContexts);
    /* Init book-keeping vectors */
    __libdata.init = true;
    return;
}


void destroy()
{
    auto& __libdata = *internal::__awc2_lib_get_instance();
    __libdata.init = false;
    __libdata.activeid = DEFAULT8;
    __libdata.ctxpool.destroy();
    __libdata.initializedContexts.resize(0);
    __libdata.inactiveContexts.resize(0);


    for(auto& context_obj : __libdata.poolmem) {
        if(context_obj.m_FlagInit == 0x1) /* destroy all that exist */
            context_obj.destroy();
    }
    util::aligned_free(__libdata.poolmem.data());
    __libdata.poolmem.destroy();
    glfwTerminate();
    return;
}


__attribute__((hot)) void newFrame()
{
    glfwPollEvents();
    return;
}


} // namespace AWC2