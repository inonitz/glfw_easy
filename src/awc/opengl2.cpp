#include "opengl2.hpp"
#include "macro.hpp"
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glbinding/AbstractFunction.h>
#include <glbinding/AbstractValue.h>
#include <glbinding-aux/logging.h>
#include <glbinding-aux/types_to_string.h>
#include <GLFW/glfw3.h>
#include "util/marker2.hpp"
#include <iostream>


namespace AWC {


static const auto before_callback_lambda = [](const glbinding::FunctionCall & call) {
    if(!call.function->isResolved()) {
        markstr("before_callback_lambda(...) => Couldn't Resolve Function Pointer\n");
        return;
    }
    

    std::cout << call.function->name() << "( ";
    for (unsigned i = 0; i < call.parameters.size() - 1; ++i) {
        std::cout << call.parameters[i].get() << ", ";
    }
    std::cout << call.parameters[call.parameters.size() - 1].get() << " )"; 
    if (call.returnValue) {
        std::cout << " => " << call.returnValue.get();
    }
    std::cout << std::endl;
    return;
};


void opengl_global_create()
{
    glbinding::addContextSwitchCallback([](glbinding::ContextHandle handle) {
        std::cout << "Activating context " << handle << "\n";
    });
    return;
}


void opengl_global_destroy() 
{
    return;
}


namespace Context::OpenGL {


void create(u8 context_id) {
    glbinding::initialize(context_id, glfwGetProcAddress, true, false);
    glbinding::setCallbackMask(glbinding::CallbackMask::Before | glbinding::CallbackMask::ParametersAndReturnValue);
    glbinding::setBeforeCallback(before_callback_lambda);
    #if AWC_OPENGL_LOG_COMMANDS == 1
        glbinding::aux::start();
    #endif
    return;
}


void destroy(u8 context_id) {
    #if AWC_OPENGL_LOG_COMMANDS == 1
        glbinding::aux::end();
    #endif
    /* 
        Not Necessarily what we want (we want resources released, not just unbinding the context) 
        But Hey, I've got nothing that's better :/
    */
    glbinding::releaseContext(context_id);
    return;
}


} // namespace Context::OpenGL


}