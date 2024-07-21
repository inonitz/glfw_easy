#include "awc2/source/internal_instance.hpp"
#include "awc2/source/internal_state.hpp"


namespace AWC2 {


inline void user_callback_func_noop(__attribute__((unused)) void* ptr) { 
    return;
}


template<class Func> void setUserCallback(Func handlerAddress) 
{
    internal::userCallbackTable* table = __scast(decltype(table), 
        &internal::__awc2_lib_get_active_context()->event_table
    );
    table->pointers[ internal::UserFuncIndexer<Func>()() ] = (handlerAddress == nullptr) ? 
        __rcast(up64, user_callback_func_noop) : 
        __rcast(up64, handlerAddress);
    return;
}


template void setUserCallback<user_callback_window_size> (user_callback_window_size  );
template void setUserCallback<user_callback_keyboard>	 (user_callback_keyboard 	 );
template void setUserCallback<user_callback_window_focus>(user_callback_window_focus );
template void setUserCallback<user_callback_mouse_pos>	 (user_callback_mouse_pos 	 );
template void setUserCallback<user_callback_mouse_button>(user_callback_mouse_button );
template void setUserCallback<user_callback_mouse_scroll>(user_callback_mouse_scroll );


} // namespace AWC2