#ifndef __AWC2_INTERNAL_USER_CALLBACK_TABLE_DEFINITION_HEADER__
#define __AWC2_INTERNAL_USER_CALLBACK_TABLE_DEFINITION_HEADER__
#include "awc2/include/event.hpp"


namespace AWC2::internal {


template<class Func> struct UserFuncIndexer {
private:

    static constexpr u8 isValidFuncTypeIndex = 
        type_trait::__is_same_type<Func, AWC2::user_callback_window_size >::value * 1 +
        type_trait::__is_same_type<Func, AWC2::user_callback_keyboard    >::value * 2 +
        type_trait::__is_same_type<Func, AWC2::user_callback_window_focus>::value * 3 +
        type_trait::__is_same_type<Func, AWC2::user_callback_mouse_pos   >::value * 4 +
        type_trait::__is_same_type<Func, AWC2::user_callback_mouse_button>::value * 5 +
        type_trait::__is_same_type<Func, AWC2::user_callback_mouse_scroll>::value * 6;

    static_assert(isValidFuncTypeIndex != 0, 
        "Function Type does not match overridable func type"
    );
public:


    constexpr u8 operator()() const { return isValidFuncTypeIndex - 1; }
};


struct alignsz(64) userCallbackTable 
{
    using framebuffer_size = AWC2::user_callback_window_size;
    using input_keys       = AWC2::user_callback_keyboard;
	using window_focused   = AWC2::user_callback_window_focus;
	using mouse_position   = AWC2::user_callback_mouse_pos;
	using mouse_input      = AWC2::user_callback_mouse_scroll;
    using mouse_scroll     = AWC2::user_callback_mouse_button; 
    DISABLE_WARNING_PUSH
    DISABLE_WARNING_NESTED_ANON_TYPES 
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
};


} // namespace AWC::internal

#endif