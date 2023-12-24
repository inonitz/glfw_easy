#ifndef __AWC_WINDOW_HEADER__
#define __AWC_WINDOW_HEADER__
#include "util/base.hpp"
#include <string>


typedef struct GLFWwindow GLFWwindow;


namespace AWC {


#define WINDOW_OPTION_FLAG_STARTUP_VISIBLE       0b00000001
#define WINDOW_OPTION_FLAG_STARTUP_FOCUSED       0b00000010
#define WINDOW_OPTION_FLAG_STARTUP_CENTER_CURSOR 0b00000100
#define WINDOW_OPTION_FLAG_RESIZABLE             0b00001000
#define WINDOW_OPTION_FLAG_BORDER                0b00010000
#define WINDOW_OPTION_FLAG_BORDERLESS            0b00000000
#define WINDOW_OPTION_FLAG_DEFAULT               0b00010111
#define WINDOW_OPTION_FRAMEBUFFER_BITS_DEFAULT     (24u << 25) | (8u << 20) | (8u << 15) | (8u << 10) | (8u << 5) | (8u << 0) /* Order is (high->low): Depth, Stencil, Red, Green, Blue, Alpha */
#define WINDOW_MAKE_OPTION(framebuffer_channels, flag_byte, refresh_rate) \
    ( __scast(u64, framebuffer_channels) | (__scast(u64, flag_byte) << 32) | (__scast(u64, refresh_rate) << 40) )

#define WINDOW_OPTION_DEFAULT \
    WINDOW_MAKE_OPTION(WINDOW_OPTION_FRAMEBUFFER_BITS_DEFAULT, WINDOW_OPTION_FLAG_DEFAULT, 60)



#define WINDOW_STATUS_MINIMIZED   0b00000001
#define WINDOW_STATUS_SIZE_CHANGE 0b00000010
#define WINDOW_STATUS_FOCUSED     0b00000100
#define WINDOW_STATUS_MINIMIZED_BIT_SHIFT   0u
#define WINDOW_STATUS_SIZE_CHANGE_BIT_SHIFT 1u
#define WINDOW_STATUS_FOCUSED_BIT_SHIFT     2u
struct pack WindowOptions {
    u32 fb_channels;
    u8  flags;
    u8  refresh;
    u8  status;
    u8  reserved;
    /*
        flags:
        0: visible_on_startup
        1: focus_on_startup
        2: cursor_on_center
        3: resizable
        4: border
        5-7: reserved
    */
    /*
        status:
        0: minimized
        1: size_change
        2: focused
        3-7: reserved
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
    GLFWwindow*   winHdl;
    WindowOptions config;
    std::string   name;
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