#include "test_func.hpp"
#include "awc/context.hpp"


int test_functionality()
{
    AWC::init();
    auto hdl = AWC::createWindow();
    AWC::initOpenGL(should_use_hdl_implicitly_since_awc_context_is_global);
    auto in  = AWC::createInputHandler(AWC::default_callbacks_table); //
    auto eventHandlerPointersArray = (type_definition_from_some_file){ functions_that_you_defined_in_diff_file }

    while(!hdl->shouldClose())
    {
        AWC::begin_frame();
        


        AWC::end_frame();
    }

    AWC::destroy();
}