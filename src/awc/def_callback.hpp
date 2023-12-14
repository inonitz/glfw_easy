#ifndef __AWC_DEFAULT_CALLBACK_FUNCTIONS_HEADER__
#define __AWC_DEFAULT_CALLBACK_FUNCTIONS_HEADER__
#include "eventdef.hpp"
#include "util/base.hpp"


inline void glfw_error_callback(
    int         error, 
    const char* description
  ) {
    LOG_ERR_FMT("GLFW_ERROR %u - %s\n", error, description);
    return;
}

#endif