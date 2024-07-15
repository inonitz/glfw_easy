#ifndef __AWC_CONTEXT_DEFINITION_HEADER__
#define __AWC_CONTEXT_DEFINITION_HEADER__
#include "window.hpp"
#include "input.hpp"
#include "event.hpp"
#include "userevent.hpp"
#include <ImGui/imgui.h>


namespace AWC {


struct AWCContext 
{
    WindowContext*            win;
    Input::InputUnit*         unit;
    Event::callbackTable*     callbacks;
    Event::userCallbackTable* usercallbacks;
    ImGuiContext*             imgui;


    static bool create(
        AWCContext& ctx,
        u8                               context_id,
        AWC::WindowOptions        const& options,
        AWC::WindowDescriptor     const& desc,
        AWC::Event::callbackTable const& override_funcs
    );
    static void destroy(
        AWCContext& ctx, 
        u8 context_id
    );


    static constexpr u64 allocationSize() 
    { 
        return sizeof(Input::InputUnit) + 
            sizeof(WindowContext) + 
            sizeof(Event::callbackTable) +
            sizeof(Event::userCallbackTable); 
    }
};


} // namespace AWC


#endif