#include "opengl2.hpp"
#include "macro.hpp"
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glbinding/AbstractFunction.h>
#include <glbinding/AbstractValue.h>
#include <glbinding-aux/debug.h>
#include <glbinding-aux/logging.h>
#include <GLFW/glfw3.h>
#include <iostream>


namespace AWC {


void opengl_global_create()
{
    static const auto after_callback_lambda = [](const glbinding::FunctionCall & call) {
        if(!call.function->isResolved()) {
            std::cout << "after_callback_lambda(...) => Couldn't Resolve Function Pointer\n";
            return;
        }


        std::cout << "[0x" << call.function->address() << "] " << call.function->name() << "\n( ";
        std::cout << call.parameters[0].get();
        for (u32 i = 0; i < call.parameters.size(); ++i) {
            std::cout << ", " << call.parameters[i].get();
        }
        std::cout << " )";

        if (call.returnValue) {
            std::cout << " ==> " << call.returnValue.get();
        }
        std::cout << "\n";
        return;
    };


    glbinding::aux::enableGetErrorCallback();
    glbinding::setCallbackMask(glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue);
    glbinding::setAfterCallback(after_callback_lambda);
    #if AWC_OPENGL_LOG_COMMANDS == 1
        glbinding::aux::start();
    #endif
    return;
}


void opengl_global_destroy() 
{
    #if AWC_OPENGL_LOG_COMMANDS == 1
        glbinding::aux::end();
    #endif
    return;
}


namespace Context::OpenGL {
    void create(u8 context_id) {
        glbinding::initialize(context_id, glfwGetProcAddress, true, false);
        return;
    }


    void destroy(u8 context_id) {
        /* 
            Not Necessarily what we want (we want resources released, not just unbinding the context) 
            But Hey, I've got nothing that's better :/
        */
        glbinding::releaseContext(context_id);
        return;
    }


} // namespace Context::OpenGL


}