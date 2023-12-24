#include "test_func.hpp"
#include "awc/context.hpp"
#include "awc/window.hpp"


int test_functionality()
{
    AWC::initialize();
    auto ctx = AWC::allocateContext();
    AWC::setActiveContext(ctx);
    AWC::initContext(1920u, 1080u, "My Very Cool Main Window", WINDOW_OPTION_DEFAULT);
    // AWC::setActiveContext(0);
    

    while(!AWC::ContextClosed()) /* <<<< */
    {
        AWC::begin_frame();
        


        AWC::end_frame();
    }


    AWC::terminate();
    return 0;
}