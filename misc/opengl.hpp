#ifndef __AWC_CONTEXT_GL_BARE_BONES_HEADER__
#define __AWC_CONTEXT_GL_BARE_BONES_HEADER__
#include <glbinding/gl/types.h>
#include "util/ifcrash.hpp"


namespace detail {


static inline gl::GLuint __glErrorCode;


constexpr const char* glErrorToString(gl::GLuint errCode) 
{
    constexpr gl::GLuint mapErrorCode[9] = {
    0, 0x0500, 0x0501, 0x502, 0x0503, 0x0504, 0x0505, 0x0506, 0xFFFFFFFF
    };
    constexpr const char* map[10] = {
        "GL_NO_ERROR",
        "GL_INVALID_ENUM",
        "GL_INVALID_VALUE",
        "GL_INVALID_OPERATION",
        "GL_STACK_OVERFLOW",
        "GL_STACK_UNDERFLOW",
        "GL_OUT_OF_MEMORY",
        "GL_INVALID_FRAMEBUFFER_OPERATION",
        "GL_INVALID_INDEX",
        "UNKOWN_GL_ERROR_CODE"
    };


    gl::GLuint i = 0;
    while(i < ( sizeof(mapErrorCode)/sizeof(mapErrorCode[0]) ) && mapErrorCode[i] != errCode) {
        ++i;
    }
    return map[i];
};


} // namespace ANON


#define __glcheck(command) \
{ \
    command; \
    detail::__glErrorCode = gl46core::glGetError(); \
    ifcrashfmt(detail::__glErrorCode, "[OPENGL] [%s] %s:%u [COMMAND] { %s } \n", detail::glErrorToString(detail::__glErrorCode), __FILE__, __LINE__, #command); \
} \


#endif

