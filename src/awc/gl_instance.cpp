#include "gl_instance.hpp"
#include "internal.hpp"


namespace AWC {


GladGLContext* getCurrentlyActiveGLContext() {
    return getActiveContext().gl;
}


} // namespace AWC