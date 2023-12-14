#include "instance.hpp"
#include "internal.hpp"

namespace AWC {


GladGLContext* getCurrentlyActiveGLContext() {
    return getActiveContext().gl;
}


WindowContext* getCurrentlyActiveWindow() {
    return getActiveContext().win;
}



} // namespace AWC