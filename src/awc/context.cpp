#include "context.hpp"
#include "internal.hpp"
#include "gl_instance.hpp"
#include "win_instance.hpp"
#include <GLFW/glfw3.h>
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include "callbackdef.hpp"



namespace AWC {


static AWCData* ginst = nullptr;


auto* __curr_gl()  { return getCurrentlyActiveGLContext(); }
auto* __curr_win() { return getCurrentlyActiveWindow(); }


void initialize() 
{
    glfwSetErrorCallback(glfw_error_callback);
    i32 result = glfwInit();

    ifcrashdo(result != GLFW_TRUE, { 
        glfwTerminate(); 
    });


    ginst = getInstance();
    ginst->__init_lib = true;
    ginst->poolAlloc.inputs.create(MAXIMUM_CONTEXTS);
    ginst->poolAlloc.handler_tables.create(MAXIMUM_CONTEXTS);
    ginst->poolAlloc.windows.create(MAXIMUM_CONTEXTS);
    ginst->poolAlloc.glctxt.create(MAXIMUM_CONTEXTS);
    ginst->contexts.reserve(MAXIMUM_CONTEXTS);
}

void terminate()
{
    if(!ginst->__init_lib) {
        debug_message("Tried to terminate library before initialization\n");
        return;
    }
    
    for(auto& ctxt : ginst->contexts) {
        memset(&ctxt.callbacks, 0x00, sizeof(Event::callbackTable));
        ctxt.unit->reset();
        ctxt.win->destroy();
        /* opengl Context can't be deleted, glfw probably manages it :/ */
    }
    glfwTerminate();
    ginst->poolAlloc.inputs.destroy();
    ginst->poolAlloc.handler_tables.destroy();
    ginst->poolAlloc.windows.destroy();
    ginst->poolAlloc.glctxt.destroy();
    ginst->contexts.resize(0);
    return;
}


void begin_frame()
{
    __curr_gl()->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void end_frame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    AWC::Input::reset();

    /* Needs to happen per-window */
    glfwPollEvents();
    glfwSwapBuffers(__curr_win()->underlying_handle());
}



u8 allocateContext()
{
    auto& galloc = ginst->poolAlloc;

    AWCData::WinContext newctxt = {
        galloc.inputs.allocate(),
        galloc.handler_tables.allocate(),
        galloc.windows.allocate(),
        galloc.glctxt.allocate()
    };
    
    
    if(newctxt.unit == nullptr 
        || newctxt.callbacks == nullptr
        || newctxt.win == nullptr
        || newctxt.gl  == nullptr
    ) {
        return 0;
    }
    ginst->contexts.push_back(newctxt);
    return ginst->contexts.size();
}

void setActiveContext(u8 id)
{
    --id;
    ifcrashfmt_debug(id >= ginst->poolAlloc.windows.size(), 
        "ID (%u) exceeds max available contexts (%u)\n", 
        id, 
        MAXIMUM_CONTEXTS
    );
    
    ginst->activeContext = id;
    glfwMakeContextCurrent(__curr_win()->underlying_handle());
    return;
}


bool initContext(
    u16 width, 
    u16 height, 
    std::string_view const& name, 
    u64 windowOptions
) {
    /* Get latest created context. */
    auto& active = ginst->contexts.back();

    /* Create the Input Unit */
    active.unit->create();
    
    /* set callback Table to default funcs */
    active.callbacks->pointers[0] = __rcast(u64, &glfw_framebuffer_size_callback);
    active.callbacks->pointers[1] = __rcast(u64, &glfw_key_callback);
    active.callbacks->pointers[2] = __rcast(u64, &glfw_window_focus_callback);
    active.callbacks->pointers[3] = __rcast(u64, &glfw_cursor_position_callback);
    active.callbacks->pointers[4] = __rcast(u64, &glfw_mouse_button_callback);
    debugnobr(
        active.callbacks->pointers[5] = __rcast(u64, &gl_debug_message_callback);
    );

    
    /* Create the GLFW Window Context */
    active.win->create(width, height, name, windowOptions);
    active.win->setCurrent();

    /* Create the OpenGL Context (last, pretty expensive) */
    return gladLoadGLContext(active.gl, glfwGetProcAddress);
}


} // namespace AWC




namespace AWC::Input {
    void create() {
        getActiveContext().unit->create();
    }
    void reset() {
        getActiveContext().unit->reset();
    }

    bool isKeyPressed(keyCode key) { 
        return getActiveContext().
            unit->getKeyState(key) == inputState::PRESS;  
    }
    bool isKeyReleased(keyCode key) { 
        return getActiveContext().
            unit->getKeyState(key) == inputState::RELEASE; 
    }
    bool isMouseButtonPressed(mouseButton but) { 
        return getActiveContext().
            unit->getMouseButtonState(but) == inputState::PRESS;   
    }
    bool isMouseButtonReleased(mouseButton but) { 
        return getActiveContext().
            unit->getMouseButtonState(but) == inputState::RELEASE;
    }

    void lockCursor() 
    {
        glfwSetInputMode(
            getActiveContext().win->underlying_handle(), 
            GLFW_CURSOR, 
            GLFW_CURSOR_DISABLED
        );
        return;
    }
    void unlockCursor()
    {
        glfwSetInputMode(
            getActiveContext().win->underlying_handle(), 
            GLFW_CURSOR, 
            GLFW_CURSOR_NORMAL
        );
        return;
    }
    void setCursorMode(bool lock)
    {
        i32 chosenMacro = (lock == true) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
        glfwSetInputMode(
            getActiveContext().win->underlying_handle(), 
            GLFW_CURSOR, 
            chosenMacro
        );
        return;
    }

} // namespace AWC::Input



namespace AWC::Event {
    template<class Func> void overrideHandler(Func* handlerAddress) {
        constexpr u8 isValidFuncTypeIndex = 
            std::is_same<Func*, GLFWframebuffersizefun>::value * 1 +
            std::is_same<Func*, GLFWkeyfun			  >::value * 2 +
            std::is_same<Func*, GLFWwindowfocusfun	  >::value * 3 +
            std::is_same<Func*, GLFWcursorposfun      >::value * 4 +
            std::is_same<Func*, GLFWmousebuttonfun	  >::value * 5 +
            std::is_same<Func*, OpenGLdbgmsgfun		  >::value * 6;
        static_assert(isValidFuncTypeIndex != 0, "Function Type does not match overridable func type");
        
        
        getActiveContext().callbacks->pointers[isValidFuncTypeIndex - 1] = __rcast(u64, handlerAddress);
        return;
    }

} // namespace AWC::Event