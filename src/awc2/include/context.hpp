#ifndef __AWC2_CONTEXT_INTERFACE_HEADER__
#define __AWC2_CONTEXT_INTERFACE_HEADER__
#include "window_types.hpp"
#include <vector>


namespace AWC2 {
    using ContextID = unsigned char;
    using ContextVector = const std::vector<u8>;
    struct ViewportSize {
        u32 x;
        u32 y;
    };


    ContextID createContext();
    void      destroyContext(ContextID id);
    void      initializeContext(
        ContextID id, 
        u16 windowWidth,
        u16 windowHeight,
        WindowDescriptor const& win_desc
    );
    void      closeContext(ContextID id);
    
    ContextVector   getActiveContextList();
    __hot bool      getContextStatus(ContextID id); /* If context is closed or not */
    __hot void      setCurrentContext(ContextID id = 0);
    __hot ContextID getCurrentContext();
    __hot ViewportSize getCurrentContextViewport();
    __hot void begin();
    __hot void end();
} // namespace AWC2


#endif