#ifndef __AWC_INSTANCE_MEMBERS_DEFINITION_HEADER__
#define __AWC_INSTANCE_MEMBERS_DEFINITION_HEADER__
#include <glad2/gl.h>
#include "window.hpp"


namespace AWC {


GladGLContext* getCurrentlyActiveGLContext();
WindowContext* getCurrentlyActiveWindow();


} // namespace AWC

#endif