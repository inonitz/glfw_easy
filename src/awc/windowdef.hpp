#ifndef __AWC_WINDOW_DEFINITION_HEADER__
#define __AWC_WINDOW_DEFINITION_HEADER__
#include "util/base.hpp"
#include <string>


typedef struct GLFWwindow GLFWwindow;


namespace AWC {


#define WINDOW_OPTION_STARTUP_VISIBLE       0b00000001
#define WINDOW_OPTION_STARTUP_FOCUSED       0b00000010
#define WINDOW_OPTION_STARTUP_CENTER_CURSOR 0b00000100
#define WINDOW_OPTION_RESIZABLE             0b00001000
#define WINDOW_OPTION_BORDER                0b00010000
#define WINDOW_OPTION_BORDERLESS            0b00000000
#define WINDOW_OPTION_DEFAULT               0b00010111
#define WINDOW_FRAMEBUFFER_BITS_DEFAULT     (24u << 25) | (8u << 20) | (8u << 15) | (8u << 10) | (8u << 5) | (8u << 0) /* Order is (high->low): Depth, Stencil, Red, Green, Blue, Alpha */


struct WindowOptions {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnested-anon-types"
    union {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
        struct pack {
            u32 fb_channels;
            u8  flags;
            u8  refresh;
            u16 reserved;
        };
        u64 bits;
        #pragma GCC diagnostic pop
    };
    #pragma GCC diagnostic pop
    /*
        Creation flags:
        0: visible_on_startup
        1: focus_on_startup
        2: cursor_on_center
        3: resizable
        4: border

        Program flags:
        5: windowWasMinimized
        6: windowSizeChanged
        7: windowIsFocused
    */
};


struct WindowDescriptor
{
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnested-anon-types"
    union {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
        struct { u32 x, y; };
        struct { u32 dims[2] = { DEFAULT32, DEFAULT32 }; };
        #pragma GCC diagnostic pop
    };
    #pragma GCC diagnostic pop
    GLFWwindow* winHdl = nullptr;
};


} // namespace AWC

#endif