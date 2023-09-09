#include "context.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include "instance.hpp"


void glfw_error_callback(
    int error, 
    const char* description
);


namespace AWC {


static AWCData* ginst = nullptr;


__force_inline AWCData::WinContext& getActiveContext() {
    return ginst->contexts[ ginst->activeContext ];
}


void init() 
{
    glfwSetErrorCallback(glfw_error_callback);
    i32 result = glfwInit();

    ifcrashdo(result != GLFW_TRUE, { 
        glfwTerminate(); 
    });


    ginst = getInstance();
    ginst->poolAlloc.inputs.create(MAXIMUM_CONTEXTS);
    ginst->poolAlloc.windows.create(MAXIMUM_CONTEXTS);
    ginst->poolAlloc.handler_tables.create(MAXIMUM_CONTEXTS);
    ginst->contexts.reserve(MAXIMUM_CONTEXTS);
}

void destroy()
{
    glfwTerminate();
    for(auto& ctxt : ginst->contexts) {
        memset(&ctxt.callbacks, 0x00, sizeof(Event::callbackTable));
        ctxt.unit->reset();
        ctxt.win->destroy();
    }
    ginst->poolAlloc.handler_tables.destroy();
    ginst->poolAlloc.inputs.destroy();
    ginst->poolAlloc.windows.destroy();
    ginst->contexts.resize(0);
    return;
}


void begin_frame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
    glfwSwapBuffers(getActiveContext().win->underlying_handle());
}



u8 allocateContext()
{
    auto* galloc = &ginst->poolAlloc;

    AWCData::WinContext newctxt = {
        galloc->windows.allocate(),
        galloc->inputs.allocate(),
        galloc->handler_tables.allocate()
    };
    
    
    if(newctxt.callbacks == nullptr 
        || newctxt.unit == nullptr 
        || newctxt.win == nullptr
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
    glfwMakeContextCurrent(getActiveContext().win->underlying_handle());
    return;
}


} // namespace AWC


void glfw_error_callback(int error, const char* description)
{
    LOG_ERR_FMT("GLFW_ERROR %u - %s\n", error, description);
    return;
}




namespace AWC::Input {
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
            getActiveContext().
                win->underlying_handle(), 
            GLFW_CURSOR, 
            GLFW_CURSOR_DISABLED
        );
        return;
    }
    void unlockCursor()
    {
        glfwSetInputMode(
            getActiveContext().
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
            getActiveContext().
                win->underlying_handle(), 
            GLFW_CURSOR, 
            chosenMacro
        );
        return;
    }

} // namespace AWC::Input



namespace AWC::Event {
    template<class Func> void overrideHandler(Func* handlerAddress) {
        constexpr u8 isValidFuncTypeIndex = 
            std::is_same<Func, GLFWframebuffersizefun>::value * 1 +
            std::is_same<Func, GLFWkeyfun			 >::value * 2 +
            std::is_same<Func, GLFWwindowfocusfun	 >::value * 3 +
            std::is_same<Func, GLFWcursorposfun		 >::value * 4 +
            std::is_same<Func, GLFWmousebuttonfun	 >::value * 5 +
            std::is_same<Func, OpenGLdbgmsgfun		 >::value * 6;
        
        static_assert(isValidFuncTypeIndex != 0, "Function Type does not match overridable func type");
        
        
        getActiveContext().callbacks->pointers[isValidFuncTypeIndex - 1] = handlerAddress;
        return;
    }

} // namespace AWC::Event