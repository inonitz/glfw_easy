#ifndef __AWC2_WINDOW_TYPES_HEADER__
#define __AWC2_WINDOW_TYPES_HEADER__
#include "util/macro.hpp"
#include "util/types.hpp"


namespace AWC2 {


enum class WindowCreationFlag : u8 {
    STARTUP_VISIBLE       = 0b00000001,
    STARTUP_FOCUSED       = 0b00000010,
    STARTUP_CENTER_CURSOR = 0b00000100,
    RESIZABLE             = 0b00001000,
    BORDER                = 0b00010000,
    BORDERLESS            = 0b00000000,
    RAW_MOUSE_MOTION      = 0b00100000,
    DEFAULT               = 0b00111101,
    MAX                   = 0b01000000
};
inline WindowCreationFlag operator&(WindowCreationFlag flagA, WindowCreationFlag flagB) {
    return __scast(WindowCreationFlag, __scast(u8, flagA) & __scast(u8, flagB) );
}
inline WindowCreationFlag operator|(WindowCreationFlag flagA, WindowCreationFlag flagB) {
    return __scast(WindowCreationFlag, __scast(u8, flagA) | __scast(u8, flagB) );
}
inline WindowCreationFlag operator&=(WindowCreationFlag& flagA, WindowCreationFlag flagB) {
    flagA = flagA & flagB;
    return flagA;
}
inline WindowCreationFlag operator|=(WindowCreationFlag& flagA, WindowCreationFlag flagB) {
    flagA = flagA | flagB;
    return flagA;
}


enum class WindowStateFlag : u8 {
    MINIMIZED    = 0b00000001,
    SIZE_CHANGED = 0b00000010,
    FOCUSED      = 0b00000100,
    MAX          = 0b00001000
};
static inline WindowStateFlag from_conditional(WindowStateFlag flagA, bool condition) {
    return __scast(WindowStateFlag, __scast(u8, flagA) * condition );
}
inline WindowStateFlag operator&(WindowStateFlag flagA, WindowStateFlag flagB) {
    return __scast(WindowStateFlag, __scast(u8, flagA) & __scast(u8, flagB) );
}
inline WindowStateFlag operator|(WindowStateFlag flagA, WindowStateFlag flagB) {
    return __scast(WindowStateFlag, __scast(u8, flagA) | __scast(u8, flagB) );
}
inline WindowStateFlag operator~(WindowStateFlag flagA) {
    return __scast(WindowStateFlag, ~__scast(u8, flagA) );
}
inline WindowStateFlag operator&=(WindowStateFlag& flagA, WindowStateFlag flagB) {
    flagA = flagA & flagB;
    return flagA;
}
inline WindowStateFlag operator|=(WindowStateFlag& flagA, WindowStateFlag flagB) {
    flagA = flagA | flagB;
    return flagA;
}

constexpr u32 bitsPerFramebufferChannel(
    u8 Depth   = 24u, 
    u8 Stencil = 8u, 
    u8 Red     = 8u, 
    u8 Green   = 8u, 
    u8 Blue    = 8u, 
    u8 Alpha   = 8u
) {
    return (Depth << 25) | (Stencil << 20) 
    | (Red   << 15) 
    | (Green << 10) 
    | (Blue  << 5) 
    | (Alpha << 0);
}


struct alignsz(8) WindowDescriptor {
    u32                framebufferChannels;
    u16                refreshRate;
    WindowCreationFlag createFlags;
    WindowStateFlag    stateFlags;


    WindowDescriptor() : 
        framebufferChannels(bitsPerFramebufferChannel()),
        refreshRate(60),
        createFlags(WindowCreationFlag::DEFAULT),
        stateFlags() {}
    
    WindowDescriptor(
        u32 framebuffer_bits_per_channel,
        u16 refresh_rate,
        WindowCreationFlag const& setup_flags
    ) :
        framebufferChannels(framebuffer_bits_per_channel),
        refreshRate(refresh_rate),
        createFlags(setup_flags),
        stateFlags() {}
    
};


} // namespace AWC2


#endif