#include "awc_internal.hpp"


namespace AWC::Context {

__hot GladGLContext const* opengl() {
    return activeContext().opengl;
}


} // namespace AWC::Context
