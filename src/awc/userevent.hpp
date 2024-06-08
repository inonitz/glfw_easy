#ifndef __AWC_USER_EVENT_CALLBACK_HEADER__
#define __AWC_USER_EVENT_CALLBACK_HEADER__
#include "usereventdef.hpp"


namespace AWC::Event {


struct alignsz(64) userCallbackTable 
{
    using framebuffer_size = user_callback_window_size;
    using input_keys       = user_callback_keyboard;
	using window_focused   = user_callback_window_focus;
	using mouse_position   = user_callback_mouse_pos;
	using mouse_input      = user_callback_mouse_scroll;
    using mouse_scroll     = user_callback_mouse_button; 

    DISABLE_WARNING_PUSH
    DISABLE_WARNING_NESTED_ANON_TYPES 
    DISABLE_WARNING_PUSH
    DISABLE_WARNING_GNU_ANON_STRUCT
    union 
    {
        struct {
            framebuffer_size windowSizeEvent;
            input_keys       keyEvent;
            window_focused   activeWinEvent;
            mouse_position   mousePosEvent;
            mouse_input      mouseButtonEvent;
            mouse_scroll     mouseScrollEvent;
            u64 reserved0[2];
        };
        struct {
            u64 pointers[8];
        };
    };
    DISABLE_WARNING_POP
    DISABLE_WARNING_POP
};


}

#endif // namespace AWC::Event