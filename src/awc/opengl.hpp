#ifndef __AWC_CONTEXT_GL_BARE_BONES_HEADER__
#define __AWC_CONTEXT_GL_BARE_BONES_HEADER__
#include "util/base.hpp"
#include "util/ifcrash.hpp"
#include <glad/gl.h>
#include <array>


namespace AWC::Context {


__hot GladGLContext const* opengl();


} // namespace AWC::Context


__force_inline auto const* gl() { return AWC::Context::opengl(); }


namespace detail {


static inline u32 __glErrorCode;


constexpr const char* glErrorToString(u32 errCode) 
{
    constexpr std::array<u32, 9> mapErrorCode = {
    0, 0x0500, 0x0501, 0x502, 0x0503, 0x0504, 0x0505, 0x0506, 0xFFFFFFFF
    };
    constexpr std::array<const char*, 10> map = {
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


    u8 i = 0;
    while(i < mapErrorCode.size() && mapErrorCode[i] != errCode) {
        ++i;
    }
    return map[i];
};


} // namespace ANON


#define __glcheck(command) { \
    command; \
    detail::__glErrorCode = AWC::Context::opengl()->GetError(); \
    ifcrashfmt(detail::__glErrorCode, "[OPENGL] [%s] %s:%u [COMMAND] { %s } \n", detail::glErrorToString(detail::__glErrorCode), __FILE__, __LINE__, #command); \
    } \


#endif

