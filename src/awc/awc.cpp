#include "awc.hpp"
#include "awc_internal.hpp"
#include "util/aligned_malloc.hpp"
#include "util/ifcrash.hpp"
#include "util/marker.hpp"
#include "userevent.hpp"
#include "input.hpp"
#include "window.hpp"
#include "def_callback.hpp"
#include <glbinding/glbinding.h>
#include <glbinding/gl/gl.h>
#include <GLFW/glfw3.h>
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"


void glfw_error_callback(
    int error, 
    const char* description
);


using ImGuiContextStructure = ImGuiContext;


namespace AWC {


void init() 
{
    ifcrashstr_debug(AWC_LIB_INITIALIZED(), 
        "AWC::init() => Tried to initialize AWC MORE THAN ONCE\n"
    );
    auto*     ginst       = getInstance();
    auto&     galloc      = ginst->poolAlloc;
    size_t    alloc_size  = 0;
    uintptr_t offset_size = 0;
    size_t max_ctxts   = ginst->contexts.size();


    glfwSetErrorCallback(glfw_error_callback);
    ifcrashdo(glfwInit() != GLFW_TRUE, { 
        glfwTerminate(); 
    });
    
    
    static constexpr size_t k_peralloc_size = 
        sizeof(Input::InputUnit) + 
        sizeof(WindowContext) + 
        sizeof(Event::callbackTable) +
        sizeof(Event::userCallbackTable); 
    alloc_size = k_peralloc_size * max_ctxts;
    debugnobr(
        ginst->poolAlloc.global_size = alloc_size;
    );
    galloc.global_shared = util::aligned_malloc<CACHE_LINE_BYTES>(alloc_size);


    offset_size = __rcast(uintptr_t, galloc.global_shared);
    ginst->poolAlloc.inputs.create(__rcast(void*, offset_size), max_ctxts);

    offset_size += galloc.inputs.bytes();
    galloc.windows.create(__rcast(void*, offset_size), max_ctxts);

    offset_size += galloc.windows.bytes();
    galloc.handler_tables.create(__rcast(void*, offset_size), max_ctxts);

    offset_size += galloc.handler_tables.bytes();
    galloc.userhandler_tables.create(__rcast(void*, offset_size), max_ctxts);


    AWC_LIB_SET_BITS(ginst->flags, AWC_LIB_INIT_MASK);
    return;
}


void destroy()
{
    auto* ginst = getInstance();
    ImGuiContextStructure* imgui_ctx_ptr = nullptr;


    /* Terminate all ACTIVE contexts */
    for(auto & context: ginst->contexts) {
        if(context.win == nullptr) /* if win == nullptr the rest are also null */
            continue;
        
        /* Shutdown ImGui Related stuff */
        imgui_ctx_ptr = __rcast(ImGuiContextStructure*, context.imgui);
        ImGui::SetCurrentContext(imgui_ctx_ptr);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(imgui_ctx_ptr);

        /* Reset/Destroy Event Handlers, Input Buffers, and Window System (GLFW) */
        memset(context.callbacks,     0x00, sizeof(Event::callbackTable));
        memset(context.usercallbacks, 0x00, sizeof(Event::userCallbackTable));
        context.unit->reset();
        context.win->destroy();
    }
    ginst->contexts.fill({});
    glfwTerminate();


    /* Destroy Memory Allocators */
    ginst->poolAlloc.userhandler_tables.destroy();
    ginst->poolAlloc.handler_tables.destroy();
    ginst->poolAlloc.windows.destroy();
    ginst->poolAlloc.inputs.destroy();
    /* Release Shared Memory Previously allocated */
    util::aligned_free(ginst->poolAlloc.global_shared);


    /* Destroy Counting Buffers used in count.hpp */
    return;
}


void begin_frame()
{
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    gl::glClear(gl::ClearBufferMask{GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT});
    return;
}

void end_frame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    /* per-window */
    glfwSwapBuffers(activeContext().win->underlying_handle());
    AWC::Input::reset();
    return;
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
        galloc->userhandler_tables.allocate(),
        __rcast(AWC::ImGuiContext*, ImGui::CreateContext())
    };
    
    
    if(newctxt.imgui == nullptr 
        || newctxt.usercallbacks == nullptr 
        || newctxt.callbacks == nullptr 
        || newctxt.unit == nullptr 
        || newctxt.win == nullptr
    ) {
        markstr("AWC::Context::allocate() => Failed To Create Context\n");
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
    i32 glver = 1;


    /* Input Buffer reset */
    active.unit->reset();
    
    /* Callback Function Table Reset/Init */
    *active.callbacks = (override_funcs.pointers[0] == 0) ? 
        AWC::Event::defaultCallbacks 
        : 
        override_funcs;
    for(auto& ptr : active.usercallbacks->pointers) {
        ptr = __rcast(u64, &user_callback_func_noop);
    }

    /* Window Init */
    active.win->create(desc, options.bits);
    active.win->setCurrent();
    active.win->setEventHooks(active.callbacks);

    /* OpenGL Init after glfw */
    glbinding::initialize(AWC_LIB_ACTIVE_CONTEXT(), glfwGetProcAddress, true, false);

    /* Init ImGui Context and Related Backends - in this case GLFW & OpenGL Backends */
    IMGUI_CHECKVERSION();
    ImGui::SetCurrentContext(__rcast(ImGuiContextStructure*, active.imgui));
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(active.win->underlying_handle(), true);
    glver = ImGui_ImplOpenGL3_Init("#version 460");
    if(!glver) {
        markstr("AWC::Context::init(...) => Couldn't initialize ImGui's OpenGL Context\n");
        return 0;
    };


#ifdef _DEBUG
    gl::glEnable(gl::GL_DEBUG_OUTPUT);
    gl::glEnable(gl::GL_DEBUG_OUTPUT_SYNCHRONOUS);
    gl::glDebugMessageControl(
        gl::GL_DONT_CARE, 
        gl::GL_DONT_CARE, 
        gl::GL_DEBUG_SEVERITY_NOTIFICATION, 
        0,
        nullptr, 
        0
    );
    gl::glDebugMessageCallback(__rcast(gl::GLDEBUGPROC, active.callbacks->openglDebugEvent), nullptr);
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
    memcpy(
        __scast(void*, size.data()), 
        __scast(void*, getInstance()->contexts[--id].win->getSize() ), 
        2 * sizeof(u32)
    );
    return size;
}


} // namespace AWC::Context




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
bool isKeyRepeated(keyCode key) {
    return activeContext().
        unit->getKeyState(key) == inputState::REPEAT; 
}
bool isMouseButtonPressed(mouseButton but) { 
    return activeContext().
        unit->getMouseButtonState(but) == inputState::PRESS;   
}
bool isMouseButtonReleased(mouseButton but) { 
    return activeContext().
        unit->getMouseButtonState(but) == inputState::RELEASE;
}
bool isMouseMoving() { 
    return activeContext().
        unit->getMouseMovementState() == true;
}
bool isMouseScrollMoving() { 
    return activeContext().
        unit->getScrollMovementState() == true;
}


std::array<f32, 2> getPreviousMousePosition() {
    return activeContext().unit->getPreviousFrameCursorPos<f32>();
}
std::array<f32, 2> getMousePosition() {
    return activeContext().unit->getCurrentFrameCursorPos<f32>();
}
std::array<f32, 2> getMouseScrollOffset() {
    return activeContext().unit->getCurrentFrameScrollOffset<f32>();
}
std::array<f32, 2> getMousePositionDelta() {
    return activeContext().unit->getCursorDelta<f32>();
}


#define repeat_common_code(mode) \
    glfwSetInputMode(activeContext().win->underlying_handle(), \
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
    activeContext().usercallbacks->pointers[UserFuncIndexer<Func>()()] = (handlerAddress == nullptr) ? 
        __rcast(uintptr_t, &user_callback_func_noop) : 
        __rcast(u64, handlerAddress);
}


template<class Func, bool isScrollFunction = false> void overrideLibraryHandler(Func* handlerAddress) {
    ifcrashstr_debug(!handlerAddress, "user-handed library handler must NOT be a nullptr (unless you want seg faults from GLFW)");
    activeContext().callbacks->pointers[AWCLibFuncIndexer<Func, isScrollFunction>()()] = __rcast(u64, handlerAddress);
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