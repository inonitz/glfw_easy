#include "test_func.hpp"
#include "awc/context.hpp"


int test_functionality()
{
    AWC::init();


    AWC::createContext();
    
        1920, 1080,
        "My Very Cool Main Window",
        WINDOW_OPTION_DEFAULT
    );
    
    AWC::setActiveContext(0);
    

    while(!win.shouldClose())
    {
        AWC::begin_frame();
        


        AWC::end_frame();
    }


    AWC::destroy();
    return 0;
}