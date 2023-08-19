#ifndef __AWC_WINDOW_HEADER__
#define __AWC_WINDOW_HEADER__
#include "util/base.hpp"


typedef struct GLFWwindow GLFWwindow;


namespace AWC {


#define WINDOW_OPTION_STARTUP_VISIBLE       0b00000001
#define WINDOW_OPTION_STARTUP_FOCUSED       0b00000010
#define WINDOW_OPTION_STARTUP_CENTER_CURSOR 0b00000100
#define WINDOW_OPTION_RESIZABLE             0b00001000
#define WINDOW_OPTION_BORDER                0b00010000
#define WINDOW_OPTION_BORDERLESS            0b00000000


struct WindowOptions
{
    u8 flags = 0b00010111;
    /*
        bit 0: visible_on_startup
        bit 1: focus_on_startup
        bit 2: cursor_on_center
        bit 3: resizable
        bit 4: border
        bit 5-7: reserved
    */
    u8 refresh_rate = 0; /* 0 <= don't care, default. */
    struct bitsPerChannel {
        u8 depth;
        u8 stencil;
        u8 rgba[4];
    } framebuffer_flags = {
        24u,
        8u,
        { 8u, 8u, 8u, 8u }
    };
};


struct WindowDescriptor
{
    union {
        u32 x, y;
        struct { u32 dims[2]; };
    };
    GLFWwindow* winHdl;
    char        name[30];
};


struct WindowContext
{
public:
    void create(
        WindowDescriptor const& props, 
        WindowOptions    const& optional
    );
    void destroy();
    void setCurrent() const;
    void setVerticalSync(u8 val) const;
    void close() const;
    bool shouldClose();

    void lockCursor() const;
    void unlockCursor();

    u32 getWidth()  const { return m_props.x; }
    u32 getHeight() const { return m_props.y; }
    
private:
    WindowDescriptor m_props;

};


}

#endif