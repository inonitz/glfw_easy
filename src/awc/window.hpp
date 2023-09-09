#ifndef __AWC_WINDOW_HEADER__
#define __AWC_WINDOW_HEADER__
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


struct pack WindowOptions {
    u32 fb_channels;
    u8  flags;
    u8  refresh;
    u16 reserved;
    /*
        flags:
        0: visible_on_startup
        1: focus_on_startup
        2: cursor_on_center
        3: resizable
        4: border
        5-7: reserved
    */
};


struct alignsz(64) WindowDescriptor
{
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnested-anon-types"
    union {
        u32 x, y;
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
        struct { u32 dims[2]; };
        #pragma GCC diagnostic pop
    };
    #pragma GCC diagnostic pop
    GLFWwindow* winHdl;
    std::string name;
};


struct WindowContext
{
public:
    bool create(
        u32 width, 
        u32 height, 
        std::string_view const& name,
        u64 windowOptions = 0
    );
    bool create(
        WindowDescriptor const& props,
        u64 windowOptions = 0
    );
    void destroy();
    void setCurrent() const;
    void setVerticalSync(u8 val) const;
    void close() const;
    bool shouldClose();

    u32 getWidth()  const { return m_props.x; }
    u32 getHeight() const { return m_props.y; }
    GLFWwindow* underlying_handle() const { return m_props.winHdl; }
private:
    bool common_create(WindowOptions options);

    WindowDescriptor m_props;

};


}

#endif