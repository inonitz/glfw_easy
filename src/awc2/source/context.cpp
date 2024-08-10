#include "include/context.hpp"
#include "include/entry.hpp"
#include "internal_instance.hpp"
#include "internal_state.hpp"
#include "util/ifcrash.hpp"
#include <algorithm>


namespace AWC2 { 


ContextID createContext() {
    auto& instance = *internal::__awc2_lib_get_instance();
    ifcrashstr_debug(instance.init == false, 
        "createContext() => AWC2 Library not initialized! call AWC2::init()\n"
    );
    return instance.ctxpool.allocate_index() + 1;
}


void destroyContext(ContextID id)
{
    ifcrashstr_debug(id == 0 || id > AWC2::k_maximumContexts, 
        "AWC2::destroyContext(ContextID id) => invalid id supplied"
    );
    internal::__awc2_lib_get_context(id).destroy();
    internal::__awc2_lib_get_instance()->ctxpool.free_index(--id);
    return;
}


void initializeContext(
    ContextID id, 
    u16 windowWidth,
    u16 windowHeight,
    WindowDescriptor const& win_desc
) {
    ifcrashstr_debug(id == 0 || id > AWC2::k_maximumContexts, 
        "AWC2::initializeContext(...) => invalid id supplied"
    );
    auto& ctxt = internal::__awc2_lib_get_context(id); 
    ctxt.create(
        id,
        windowWidth, 
        windowHeight, 
        win_desc
    );
    ctxt.initialize();
    internal::__awc2_lib_get_instance()->initializedContexts.push_back(id);
    return;
}


void closeContext(ContextID id)
{
    /*
        [NOTE]: 
        Closing GLFW windows dynamically doesn't work,
        dynamically meaning in the game-loop, even if the window/its context is unbound;
        (4 days of my life are gone :( )
        Therefore, the only thing that can be done is to hide it and have a Flag to notify the
        programmer that this windows' context is effectively unusable.
        Also, waiting until AWC2::destroy() to completely release all the windows I previously allocated :/
    */
    auto& instance = *internal::__awc2_lib_get_instance();

    internal::__awc2_lib_get_context(id).close();
    instance.initializedContexts.erase(std::find(
        instance.initializedContexts.begin(), 
        instance.initializedContexts.end(),
        id
    ));
    instance.inactiveContexts.push_back(id);
    return;
}


ContextVector getActiveContextList()
{
    return internal::__awc2_lib_get_instance()->initializedContexts;
}


__hot bool getContextStatus(ContextID id) {
    return internal::__awc2_lib_get_context(id).m_FlagClosed;
}


__hot void setCurrentContext(ContextID id) 
{
    ifcrashstr_debug(id > AWC2::k_maximumContexts, 
        "AWC2::initializeContext(...) => invalid id supplied"
    );

    if(unlikely(id == 0)) {
        /* unbind everthing */
        /* No way to avoid this as order-of-operations is opposite from binding */
        internal::unbindActiveContext();
    } else {
        internal::__awc2_lib_get_context(id).setCurrent();
    }
    internal::__awc2_lib_get_instance()->activeid = (id == 0) ? 0xFF : id;
    return;
}


__hot ContextID getCurrentContext()
{
    return internal::__awc2_lib_get_active_context_id();
}


__hot ViewportSize getCurrentContextViewport()
{
    return ViewportSize{
        internal::__awc2_lib_get_active_context().m_window.getWidth(),
        internal::__awc2_lib_get_active_context().m_window.getHeight()
    };
}


__hot void begin() { internal::__awc2_lib_get_active_context().begin(); return; }
__hot void end()   { internal::__awc2_lib_get_active_context().end();   return; }


} // namespace AWC2