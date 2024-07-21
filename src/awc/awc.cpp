#include "awc.hpp"
#include "contextdef.hpp"
#include "instance.hpp"
#include "macro.hpp"
#include "state.hpp"
#include "opengl2.hpp"
#include "def_callback.hpp"
#include "util/ifcrash.hpp"
#include "util/marker2.hpp"
#include "util/aligned_malloc.hpp"
#include <GLFW/glfw3.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/imgui_impl_glfw.h>


void glfw_error_callback(
    int error, 
    const char* description
);


namespace AWC {


void init() 
{
    ifcrashstr_debug(__awc_lib_initialized(),
        "AWC::init() => Tried to initialize AWC MORE THAN ONCE\n"
    );
    auto*     ginst       = __get_instance();
    auto&     galloc      = ginst->mempool;
    size_t    alloc_size  = 0;
    uintptr_t offset_size = 0;
    size_t max_ctxts   = ginst->contexts.size();


    glfwSetErrorCallback(glfw_error_callback);
    ifcrashdo(glfwInit() != GLFW_TRUE, { 
        glfwTerminate(); 
    });
    IMGUI_CHECKVERSION();

    
    alloc_size = AWCContext::allocationSize() * max_ctxts;
    debugnobr(
        ginst->mempool.global_size = alloc_size;
    );
    galloc.global_shared = util::aligned_malloc<CACHE_LINE_BYTES>(alloc_size);


    offset_size = __rcast(uintptr_t, galloc.global_shared);
    ginst->mempool.inputs.create(__rcast(void*, offset_size), max_ctxts);

    offset_size += galloc.inputs.bytes();
    galloc.windows.create(__rcast(void*, offset_size), max_ctxts);

    offset_size += galloc.windows.bytes();
    galloc.handler_tables.create(__rcast(void*, offset_size), max_ctxts);

    offset_size += galloc.handler_tables.bytes();
    galloc.userhandler_tables.create(__rcast(void*, offset_size), max_ctxts);


    AWC::opengl_global_create();
    AWC_LIB_SET_BITS(ginst->flags, AWC_LIB_INIT_MASK);
    return;
}


void destroy()
{
    auto* ginst = __get_instance();
    /* Terminate all ACTIVE contexts */
    for(u8 i = 0; i < ginst->contexts.size(); ++i) 
    {
        if( ginst->contexts[i].win == nullptr )
            continue;

        AWC::AWCContext::destroy(ginst->contexts[i], i+1);
    }
    ginst->contexts.fill({});

    AWC::opengl_global_destroy();
    glfwTerminate();


    /* Destroy Memory Allocators */
    ginst->mempool.userhandler_tables.destroy();
    ginst->mempool.handler_tables.destroy();
    ginst->mempool.windows.destroy();
    ginst->mempool.inputs.destroy();
    /* Release Shared Memory Previously allocated */
    util::aligned_free(ginst->mempool.global_shared);
    return;
}


void begin_frame()
{
    glfwPollEvents();
    return;
}

void end_frame() {
    return;
}


} // namespace AWC


void glfw_error_callback(int error, const char* description)
{
    markfmt("GLFW_ERROR %u - %s\n", error, description);
    return;
}


namespace AWC::Input {


void reset() {
    __active_context()->unit->reset();
}


bool isKeyPressed(keyCode key)  { return __active_context()->unit->getKeyState(key) == inputState::PRESS;   }
bool isKeyReleased(keyCode key) { return __active_context()->unit->getKeyState(key) == inputState::RELEASE; }
bool isKeyRepeated(keyCode key) { return __active_context()->unit->getKeyState(key) == inputState::REPEAT;  }
bool isMouseButtonPressed(mouseButton but)  { return __active_context()->unit->getMouseButtonState(but) == inputState::PRESS;   }
bool isMouseButtonReleased(mouseButton but) { return __active_context()->unit->getMouseButtonState(but) == inputState::RELEASE; }
bool isMouseMoving()       { return __active_context()->unit->getMouseMovementState()  == true; }
bool isMouseScrollMoving() { return __active_context()->unit->getScrollMovementState() == true; }


std::array<f32, 2> getPreviousMousePosition() { return __active_context()->unit->getPreviousFrameCursorPos<f32>();   }
std::array<f32, 2> getMousePosition()         { return __active_context()->unit->getCurrentFrameCursorPos<f32>();    }
std::array<f32, 2> getMouseScrollOffset()     { return __active_context()->unit->getCurrentFrameScrollOffset<f32>(); }
std::array<f32, 2> getMousePositionDelta()    { return __active_context()->unit->getCursorDelta<f32>();              }


#define repeat_common_code(mode) \
    glfwSetInputMode(__active_context()->win->underlying_handle(), \
        GLFW_CURSOR, mode \
    ); \
    return; \

void unrestrictCursor() { repeat_common_code(GLFW_CURSOR_DISABLED); /* WINDOW_CURSOR_HIDDEN_VIRTUAL     */ }
void unlockCursor()     { repeat_common_code(GLFW_CURSOR_NORMAL)    /* WINDOW_CURSOR_VISIBLE_UNRESTRICT */ }
void restrictCursor()   { repeat_common_code(GLFW_CURSOR_CAPTURED)  /* WINDOW_CURSOR_VISIBLE_RESTRICT   */ }
void hideCursor()       { repeat_common_code(GLFW_CURSOR_HIDDEN)    /* WINDOW_CURSOR_HIDDEN_UNRESTRICT  */ }
void setCursorMode(u8 cursorMode)
{
    ifcrashstr_debug(cursorMode > WINDOW_CURSOR_VISIBLE_RESTRICT, 
        "setCursorMode(u8 mode) => mode variable expected to be in range"
    );
    static constexpr u32 modeToValue[4] = {
        GLFW_CURSOR_HIDDEN,
        GLFW_CURSOR_NORMAL,
        GLFW_CURSOR_DISABLED,
        GLFW_CURSOR_CAPTURED
    };
    repeat_common_code(modeToValue[cursorMode]);
}

#undef repeat_common_code


} // namespace AWC::Input




namespace AWC::Event {


template<class Func, bool isScroll = false> struct AWCLibFuncIndexer {
    static constexpr u8 isValidFuncTypeIndex = 
        std::is_same<Func, GLFWframebuffersizefun>::value * 1 +
        std::is_same<Func, GLFWkeyfun			 >::value * 2 +
        std::is_same<Func, GLFWwindowfocusfun	 >::value * 3 +
        std::is_same<Func, GLFWcursorposfun		 >::value * !isScroll * 4 +
        std::is_same<Func, GLFWmousebuttonfun	 >::value * 5 +
        std::is_same<Func, GLFWscrollfun		 >::value *  isScroll * 6 +
        std::is_same<Func, OpenGLdbgmsgfun		 >::value * 7;
    
    static_assert(isValidFuncTypeIndex != 0, 
        "Function Type does not match overridable func type"
    );

    constexpr u8 operator()() const { return isValidFuncTypeIndex - 1; }
};


template<class Func> struct UserFuncIndexer {
    static constexpr u8 isValidFuncTypeIndex = 
        std::is_same<Func, user_callback_window_size >::value * 1 +
        std::is_same<Func, user_callback_keyboard    >::value * 2 +
        std::is_same<Func, user_callback_window_focus>::value * 3 +
        std::is_same<Func, user_callback_mouse_pos   >::value * 4 +
        std::is_same<Func, user_callback_mouse_button>::value * 5 +
        std::is_same<Func, user_callback_mouse_scroll>::value * 6;

    static_assert(isValidFuncTypeIndex != 0, 
        "Function Type does not match overridable func type"
    );

    constexpr u8 operator()() const { return isValidFuncTypeIndex - 1; }
};


template<class Func> void setUserCallback(Func handlerAddress) {
    __active_context()->usercallbacks->pointers[UserFuncIndexer<Func>()()] = (handlerAddress == nullptr) ? 
        __rcast(uintptr_t, &user_callback_func_noop) : 
        __rcast(u64, handlerAddress);
}


template<class Func, bool isScrollFunction = false> void overrideLibraryHandler(Func* handlerAddress) {
    ifcrashstr_debug(!handlerAddress, "user-handed library function-callback must NOT be a nullptr (unless you want seg faults from GLFW)");
    __active_context()->callbacks->pointers[AWCLibFuncIndexer<Func, isScrollFunction>()()] = __rcast(u64, handlerAddress);
    return;
}


template<class Func, bool isScrollFunction> void resetLibraryHandler() {
    overrideLibraryHandler<Func, isScrollFunction>(
        reinterpret_cast<Func*>(
            AWC::Event::defaultCallbacks.pointers[AWCLibFuncIndexer<Func, isScrollFunction>()()]
        )
    );
    return;
}


template void setUserCallback<user_callback_window_size> (user_callback_window_size  );
template void setUserCallback<user_callback_keyboard>	 (user_callback_keyboard 	 );
template void setUserCallback<user_callback_window_focus>(user_callback_window_focus );
template void setUserCallback<user_callback_mouse_pos>	 (user_callback_mouse_pos 	 );
template void setUserCallback<user_callback_mouse_button>(user_callback_mouse_button );
template void setUserCallback<user_callback_mouse_scroll>(user_callback_mouse_scroll );

template void overrideLibraryHandler<GLFWframebuffersizefun, false>(GLFWframebuffersizefun*);
template void overrideLibraryHandler<GLFWkeyfun		   	   , false>(GLFWkeyfun*);
template void overrideLibraryHandler<GLFWwindowfocusfun	   , false>(GLFWwindowfocusfun*);
template void overrideLibraryHandler<GLFWcursorposfun  	   , false>(GLFWcursorposfun*);
template void overrideLibraryHandler<GLFWmousebuttonfun	   , false>(GLFWmousebuttonfun*);
template void overrideLibraryHandler<GLFWscrollfun         , true >(GLFWscrollfun*);
template void overrideLibraryHandler<OpenGLdbgmsgfun       , false>(OpenGLdbgmsgfun*);

template void resetLibraryHandler<GLFWframebuffersizefun, false>();
template void resetLibraryHandler<GLFWkeyfun		   	, false>();
template void resetLibraryHandler<GLFWwindowfocusfun	, false>();
template void resetLibraryHandler<GLFWcursorposfun  	, false>();
template void resetLibraryHandler<GLFWmousebuttonfun	, false>();
template void resetLibraryHandler<GLFWscrollfun	   	    , true >();
template void resetLibraryHandler<OpenGLdbgmsgfun	    , false>();


} // namespace AWC::Event