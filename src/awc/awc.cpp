#include "awc.hpp"
#include "awc_internal.hpp"
#include <GLFW/glfw3.h>
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include "util/count.hpp"
#include "def_callback.hpp"
#include "input.hpp"
#include "window.hpp"
#include "opengl.hpp"


void glfw_error_callback(
    int error, 
    const char* description
);


namespace AWC {


void init() 
{
    ifcrashfmt_debug(AWC_LIB_INITIALIZED(), 
        "AWC::init() => Tried to initialize AWC MORE THAN ONCE%c", '\n'
    );
    auto*     ginst       = getInstance();
    size_t    alloc_size  = 0;
    uintptr_t offset_size = 0;
    size_t    max_ctxts   = ginst->contexts.size();


    CREATE_LOCAL_COUNTER_BUFFER(16);
    glfwSetErrorCallback(glfw_error_callback);
    ifcrashdo(glfwInit() != GLFW_TRUE, { 
        glfwTerminate(); 
    });
    
    
    alloc_size = 
        sizeof(Input::InputUnit) + 
        sizeof(WindowContext) + 
        sizeof(Event::callbackTable) +
        sizeof(AWCData::CachedGLContext); 
    alloc_size *= max_ctxts;
    debugnobr(
        ginst->poolAlloc.global_size = alloc_size;
    );
    ginst->poolAlloc.global_shared = amalloc_t(byte, alloc_size, CACHE_LINE_BYTES);


    offset_size = __rcast(uintptr_t, ginst->poolAlloc.global_shared);
    ginst->poolAlloc.inputs.create(__rcast(void*, offset_size), max_ctxts);

    offset_size += ginst->poolAlloc.inputs.bytes();
    ginst->poolAlloc.windows.create(__rcast(void*, offset_size), max_ctxts);

    offset_size += ginst->poolAlloc.windows.bytes();
    ginst->poolAlloc.handler_tables.create(__rcast(void*, offset_size), max_ctxts);

    offset_size += ginst->poolAlloc.handler_tables.bytes();
    ginst->poolAlloc.gl.create(__rcast(void*, offset_size), max_ctxts);


    AWC_LIB_SET_BITS(ginst->flags, AWC_LIB_INIT_MASK);
    return;
}


void destroy()
{
    auto* ginst = getInstance();


    /* Terminate all ACTIVE contexts */
    for(auto & context: ginst->contexts) {
        if(context.win == nullptr) /* if win == nullptr the rest are also null */
            continue;
        
        /* Shutdown ImGui Related stuff */
        ImGui::SetCurrentContext(context.imgui);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(context.imgui);

        /* Reset/Destroy Event Handler, Input Buffers, and Window System (GLFW) */
        memset(context.callbacks, 0x00, sizeof(Event::callbackTable));
        context.unit->reset();
        context.win->destroy();
    }
    ginst->contexts.fill({});
    glfwTerminate();


    /* Destroy Memory Allocators */
    ginst->poolAlloc.gl.destroy();
    ginst->poolAlloc.handler_tables.destroy();
    ginst->poolAlloc.windows.destroy();
    ginst->poolAlloc.inputs.destroy();
    /* Release Shared Memory Previously allocated */
    afree_t(ginst->poolAlloc.global_shared);


    /* Destroy Counting Buffers used in count.hpp */
    DESTROY_LOCAL_COUNTER_BUFFER();
    return;
}


void begin_frame()
{
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    

    gl()->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void end_frame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    /* per-window */
    glfwSwapBuffers(activeContext().win->underlying_handle());
    AWC::Input::reset();
}

} // namespace AWC


void glfw_error_callback(int error, const char* description)
{
    LOG_ERR_FMT("GLFW_ERROR %u - %s\n", error, description);
    return;
}




namespace AWC::Context {
    u8 allocate()
    {
        auto* glibinst = getInstance();
        ifcrashfmt_debug( AWC_LIB_CONTEXT_COUNT() == (AWC_LIB_CONTEXT_MAX),
            "AWC::Context::allocate() => Maximum amunt of Contexts allocated%c", '\n'
        );
        auto* galloc = &glibinst->poolAlloc;

        AWCData::WinContext newctxt = {
            galloc->windows.allocate(),
            galloc->inputs.allocate(),
            galloc->handler_tables.allocate(),
            &galloc->gl.allocate()->gl,
            ImGui::CreateContext()
        };
        
        
        if(newctxt.imgui == nullptr 
            || newctxt.opengl == nullptr 
            || newctxt.callbacks == nullptr 
            || newctxt.unit == nullptr 
            || newctxt.win == nullptr
        ) {
            debug_message("AWC::Context::allocate() => Failed To Create Context\n");
            return 0;
        }


        u8 newCount = AWC_LIB_CONTEXT_COUNT();
        glibinst->contexts[newCount] = newctxt;


        AWC_LIB_MODIFY_VAR_BITS(glibinst->flags, 
            AWC_LIB_CONTEXT_COUNT_MASK, 
            ++newCount << AWC_LIB_CONTEXT_COUNT_SHIFT
        );
        return newCount;
    }


    bool init(
        AWC::WindowOptions        const& options,
        AWC::WindowDescriptor     const& desc,
        AWC::Event::callbackTable const& override_funcs
    ) {
        auto active = activeContext();
        i32 glver = 0;


        /* Input Buffer reset */
        active.unit->reset();
        
        /* Callback Function Table Reset/Init */
        *active.callbacks = (override_funcs.pointers[0] == NULL) ? 
            AWC::Event::defaultCallbacks 
            : 
            override_funcs;

        /* Window Init */
        active.win->create(desc, options.bits);
        active.win->setCurrent();
        active.win->setEventHooks(active.callbacks);

        /* OpenGL Init after glfw */
        glver = gladLoadGLContext(active.opengl, glfwGetProcAddress);
        if(!glver) {
            debug_message("AWC::Context::init() => Couldn't initialize OpenGL Context\n");
            return 0;
        }

        /* Init ImGui Context and Related Backends - in this case GLFW & OpenGL Backends */
        IMGUI_CHECKVERSION();
        ImGui::SetCurrentContext(active.imgui);
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        
        ImGui_ImplGlfw_InitForOpenGL(active.win->underlying_handle(), true);
        glver = ImGui_ImplOpenGL3_Init("#version 460");
        if(!glver) {
            debug_message("AWC::Context::init() => Couldn't initialize ImGui's OpenGL Context\n");
            return 0;
        };


#ifdef _DEBUG
        active.opengl->Enable(GL_DEBUG_OUTPUT);
        active.opengl->Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        active.opengl->DebugMessageControl(
            GL_DONT_CARE, 
            GL_DONT_CARE, 
            GL_DEBUG_SEVERITY_NOTIFICATION, 
            0, 
            nullptr, 
            GL_FALSE
        );
        active.opengl->DebugMessageCallback(active.callbacks->openglDebugEvent, nullptr);
#endif


        AWC_LIB_MODIFY_VAR_BITS(getInstance()->flags, 
            AWC_LIB_ATLEAST_ONE_CONTEXT_MASK, 
            AWC_LIB_ATLEAST_ONE_CONTEXT_MASK
        );
        return 1;
    }


    void setActive(u8 id)
    {
        ifcrashfmt_debug( AWC_LIB_CONTEXT_COUNT() < id,
            "AWC::Context::setActive() => ID %u exceeds Currently Allocated Context Amount (%u)", 
            id, AWC_LIB_CONTEXT_MAX
        );

        
        AWC_LIB_MODIFY_VAR_BITS(getInstance()->flags, 
            AWC_LIB_ACTIVE_CONTEXT_MASK, 
            (id << AWC_LIB_ACTIVE_CONTEXT_SHIFT)
        );
        return;
    }



    bool windowActive(u8 id)
    {
        return !getInstance()->contexts[--id].win->shouldClose();
    }


    std::array<u32, 2> windowSize(u8 id) 
    {
        std::array<u32, 2> size = { 0, 0 };
        memcpy(size.begin(), getInstance()->contexts[--id].win->getSize(), 2 * sizeof(u32));
        return size;
    }


    GladGLContext* gl() {
        return activeContext().opengl;
    }
}




namespace AWC::Input {
    void reset() {
        activeContext().unit->reset();
    }

    bool isKeyPressed(keyCode key) { 
        return activeContext().
            unit->getKeyState(key) == inputState::PRESS;  
    }
    bool isKeyReleased(keyCode key) { 
        return activeContext().
            unit->getKeyState(key) == inputState::RELEASE; 
    }
    bool isMouseButtonPressed(mouseButton but) { 
        return activeContext().
            unit->getMouseButtonState(but) == inputState::PRESS;   
    }
    bool isMouseButtonReleased(mouseButton but) { 
        return activeContext().
            unit->getMouseButtonState(but) == inputState::RELEASE;
    }

    void lockCursor() 
    {
        glfwSetInputMode(
            activeContext().
                win->underlying_handle(), 
            GLFW_CURSOR, 
            GLFW_CURSOR_DISABLED
        );
        return;
    }
    void unlockCursor()
    {
        glfwSetInputMode(
            activeContext().
                win->underlying_handle(), 
            GLFW_CURSOR, 
            GLFW_CURSOR_NORMAL
        );
        return;
    }
    void setCursorMode(bool lock)
    {
        i32 chosenMacro = (lock == true) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
        glfwSetInputMode(
            activeContext().
                win->underlying_handle(), 
            GLFW_CURSOR, 
            chosenMacro
        );
        return;
    }

} // namespace AWC::Input



namespace AWC::Event {
    template<class Func> class FuncIndexer {
        static constexpr u8 isValidFuncTypeIndex = 
            std::is_same<Func, GLFWframebuffersizefun>::value * 1 +
            std::is_same<Func, GLFWkeyfun			 >::value * 2 +
            std::is_same<Func, GLFWwindowfocusfun	 >::value * 3 +
            std::is_same<Func, GLFWcursorposfun		 >::value * 4 +
            std::is_same<Func, GLFWmousebuttonfun	 >::value * 5 +
            std::is_same<Func, OpenGLdbgmsgfun		 >::value * 6;
        
        static_assert(isValidFuncTypeIndex != 0, 
            "Function Type does not match overridable func type"
        );

        constexpr u8 operator()() const { return isValidFuncTypeIndex - 1; }
    };


    template<class Func> void overrideHandler(Func* handlerAddress) {
        activeContext().callbacks->pointers[FuncIndexer<Func>()()] = handlerAddress;
        return;
    }


    template<class Func, bool nullptrOrDefault> void resetHandler() {
        constexpr Func* resulting_value = nullptrOrDefault ? 
        nullptr : 
            &AWC::Event::defaultCallbacks
                .pointers[FuncIndexer<Func>()()];
        
        overrideHandler(resulting_value);
        return;
    }

} // namespace AWC::Event