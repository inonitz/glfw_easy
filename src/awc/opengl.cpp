#include "awc_internal.hpp"


namespace AWC::Context {

GladGLContext const* opengl() {
    return activeContext().opengl;
}


} // namespace AWC::Context
