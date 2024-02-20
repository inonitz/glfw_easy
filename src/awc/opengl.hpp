#ifndef __AWC_CONTEXT_GL_BARE_BONES_HEADER__
#define __AWC_CONTEXT_GL_BARE_BONES_HEADER__
#include "util/base.hpp"
#include <glad/gl.h>


namespace AWC::Context {


__hot GladGLContext const* opengl();


} // namespace AWC::Context


__force_inline auto const* gl() { return AWC::Context::opengl(); }


#endif

