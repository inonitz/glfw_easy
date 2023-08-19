#include "context.hpp"
#include <GLFW/glfw3.h>


void glfw_error_callback(
    int error, 
    const char* description
);


namespace AWC {


void init() 
{
    glfwSetErrorCallback(glfw_error_callback);
    i32 result = glfwInit();

    ifcrashdo(result != GLFW_TRUE, { glfwTerminate(); });
}


void destroy()
{
    glfwTerminate();
    return;
}


} // namespace AWC


void glfw_error_callback(int error, const char* description)
{
    LOG_ERR_FMT("GLFW_ERROR %u - %s\n", error, description);
    return;
}